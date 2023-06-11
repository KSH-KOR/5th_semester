#include <unistd.h>
#include <stdio.h>
#include "bmalloc.h" 
#include <sys/mman.h>
#include <math.h>
#include <stdlib.h>

/*
hp: header pointer
shp: sibling header pointer
nhp: next header pointer
npp: new page pointer
*/

bm_option bm_mode = BestFit ;
bm_header bm_list_head = {0, 0, 0x0 } ;

#define IN_USED 1
#define NOT_USED 0
#define PAGE_SIZE 12

void * get_size_ptr(bm_header_ptr ptr){
	return ((void *) ptr) + sizeof(bm_header);
}
void * get_header_ptr(void * ptr){
	return ((void *) ptr) - sizeof(bm_header);
}

void * get_page(){
	bm_header_ptr npp = mmap ( NULL, pow(2, PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );
	if(npp == MAP_FAILED){
		perror("mapping failed");
	}
	npp->next = (bm_header_ptr)0x0;
	npp->used = (unsigned int)0;
	npp->size = (unsigned int)PAGE_SIZE;
	return npp;
}

// return value: new page address
void * request_empty_page(){
	// NULL : let kernel choose address to map a page in the address space.
	// MAP_PRIVATE : the page is for a heap memory of the process should be private to other porcesses
	bm_header_ptr npp = get_page();

	if(bm_list_head.next == 0x0) bm_list_head.next = npp;
	else{
		bm_header_ptr last = bm_list_head.next;
		while(last->next != 0x0){
			last = last->next;
		}
		last->next = npp;
	}
	 
	return npp;
}

void reclaim_unuse_page(bm_header_ptr header){
	bm_header_ptr hp ;
	bm_header_ptr prev ;
	int i ;
	for (hp = bm_list_head.next, prev = &bm_list_head, i = 1; hp != 0x0 ; hp = hp->next, prev = prev->next, i++) {
		if(header == hp){ //found target header in the linked list
			prev->next = header->next;
		}
	}
	int err = munmap(header, pow(2, PAGE_SIZE));
	if(err != 0) perror("unmapping failed");
}

void * sibling (void * h)
{
	bm_header_ptr hp ;
	bm_header_ptr prev ;
	bm_header_ptr target = h;
	int size = target->size;
	int count = 0;
	for(hp = bm_list_head.next, prev = &bm_list_head; hp != 0x0; hp = hp->next, prev = prev->next){
		if(hp->size == size) count++;
		else count = 0;
		if(hp == target){
			if(count % 2 == 0){
				return prev->size == hp->size ? prev : 0x0;
			} else{
				return (hp->next != NULL) ? (hp->next->size == hp->size ? hp->next : 0x0) : 0x0;
			}
		}
	}
	return 0x0;
}

void * get_splited_address(void * p){
	bm_header_ptr hp = p;
	size_t size_in_bytes = pow(2, hp->size);
	return p + size_in_bytes/2;
}

void * split(void * p){
	bm_header_ptr hp = p;
	if(hp->size == 4) return hp; // block is atomic

	bm_header_ptr next = hp->next;
	bm_header_ptr new_node_hp = get_splited_address(hp);
	new_node_hp->used = 0;
	new_node_hp->size = --hp->size;
	new_node_hp->next = next; // connect new node with the next node

	hp->next = 	new_node_hp; // connect original node with the new node
	return hp;
}

// find fitting node
void * fitting (size_t s) 
{
	bm_header_ptr hp ;
	bm_header_ptr best_block_ptr = 0x0;
	size_t best_fit_size = PAGE_SIZE + 1;

	// 지금 링크드 리스트에 있는 블럭들 중에 bestfit면 요청된 사이즈를 서비스할 수 있는 가장 작은 블럭을 고릅니다. 
	for (hp = bm_list_head.next; hp != 0x0 ; hp = hp->next) {
		if(	hp->used == NOT_USED 
			&& 
			pow(2, hp->size) > s 
			&& 
			(
				(bm_mode == BestFit) 
				?
				hp->size < best_fit_size 
				: 
				1
			)
			){
			best_block_ptr = hp;
			best_fit_size = hp->size;
			if(bm_mode == FirstFit) break;
		}
	}

	// 못 찾았다면 지금 링크드리스트에는 요청된 사이즈를 서비스할 수 있는 블럭이 없다는 뜻. 새로운 페이지 필요.
	if(best_block_ptr == 0x0){
		printf("need another page!\n");
		best_block_ptr = request_empty_page();
		best_fit_size = PAGE_SIZE;
	}	

	// 앞에 for문에서 고른 block을 가능한 작게 만들어주는 과정입니다.
	while(s < pow(2, --best_fit_size)){ // equal size cannot support it since it has to use 9 bytes for header.
		best_block_ptr = split(best_block_ptr);
	}

	return best_block_ptr;
}


void * mergex(void * p, void * q){
	printf("merging.. between %p and %p\n", p, q);
	bm_header_ptr ph = p;
	bm_header_ptr qh = q;
	
	if(ph->next == qh){ // didn't erase sibling block data. can raise security problem (data leak)
		ph->next = qh->next;
		ph->size ++; 
		return ph;
	} else if(qh->next == ph){
		qh->next = ph->next;
		qh->size ++; 
		return qh;
	}
	printf("error: %p and %p are not sibling\n", ph, qh);
	abort();
	return 0x0 ;
}

// check if it has sibling
// check if its sibling is unused
void * merge(void * p){
	bm_header_ptr ph = p;
	bm_header_ptr sh;
	if((sh = sibling(ph)) == 0x0){
		return 0x0; // no sibling found, cannot merge.
	}
	if(sh->used == IN_USED){
		return 0x0; // sibling is in used, cannot merge.
	}
	return mergex(ph, sh);
}


void * bmalloc (size_t s) 
{
	bm_header_ptr hp = fitting(s);
	hp->used = 1;
	return get_size_ptr(hp) ;
}

//check if freed block is a full size block (size == page_size)
void bfree (void * p) 
{	
	bm_header_ptr hp = get_header_ptr(p);
	if(hp == 0x0 || hp->used == NOT_USED) return;
	hp->used = NOT_USED;
	if(hp->size == PAGE_SIZE){
		reclaim_unuse_page(hp); // nothing to merge
		return;
	}
	
	while(hp != 0x0){
		hp = merge(hp);
	} // loop ends when either the sibling of hp does not exist or is in used

}

void * brealloc (void * p, size_t s) 
{
	// If ptr is a NULL pointer, then brealloc behaves like malloc and allocates a new block of memory of the requested size.
	if(p == NULL){
		return bmalloc(s);
	}
	//If size is zero, then brealloc behaves like free and frees the memory pointed to by ptr.
	if(s == 0){
		bfree(p);
		return 0x0;
	}

	bm_header_ptr hp = p;
	
	// If the new size is twice smaller than the current block, assign smaller block.
	if(hp->size-1 > s){ // cannot be the same bc of the header space	
		return split(hp);
	} else if(hp->size+1 < s){ // If the new size is twice larger than the current block, then 

	} else{ // If the new size requires the same block size as the current one, then do nothing.
		return hp;
	}
	// If the new size is larger than the old size, then realloc attempts to extend the block of memory in place,
	// if it cannot do so, it allocates a new block of memory and copies the contents of the old block to the new block

	// If the allocation fails, realloc returns NULL and the old block of memory is unchanged.
	return 0x0 ;
}

void bmconfig (bm_option opt) 
{
	// If bm_option is invalid, nothing changed.
	if(opt > 1 || opt < 0) return;

	//set the space management scheme as BestFit, or FirstFit.
	bm_mode = opt;
}


void 
bmprint () 
{
	bm_header_ptr itr ;
	int i ;
	size_t total_user_given_amount = 0;
	size_t total_available_amount = 0;
	size_t total_given_amount = 0;

	printf("==================== bm_list ====================\n") ;
	for (itr = bm_list_head.next, i = 0 ; itr != 0x0 ; itr = itr->next, i++) {
		if(itr->used == 1) total_user_given_amount += pow(2, itr->size);
		else total_available_amount += pow(2, itr->size);
		total_given_amount += pow(2, itr->size);
		printf("%3d:%p:%1d %8d:", i, get_size_ptr(itr), (int)itr->used, (int) itr->size) ;
		
		int j ;
		char * s = get_size_ptr(itr) ;
		for (j = 0 ; j < (itr->size >= 8 ? 8 : itr->size) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("=================================================\n") ;

	//TODO: print out the stat's.
	if(total_user_given_amount + total_available_amount != total_given_amount) printf("something is wrong");

	printf("the total amount of all given memory: %zu\n", total_given_amount);
	printf("the total amount of memory given to the users: %zu\n", total_user_given_amount);
	printf("the total amount of available memory,: %zu\n", total_available_amount);
	
}
