#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>

#define MAX_PATH_LENGTH 256

int ignore_size = 0;

typedef struct Node {
    char path[MAX_PATH_LENGTH];
    struct Node* prev;
    struct Node* next;
} Node;

typedef Node* NodePtr;

typedef struct Subtask { // n!
    NodePtr path1; // pivot
    NodePtr path2; // compare file
} Subtask;

typedef Subtask* SubtaskPtr;

typedef struct Answer_node {
    char path[MAX_PATH_LENGTH];
    struct Answer_node * node_next;
    struct Answer_node * header_next;
} Answer_node;

typedef Answer_node* Answer_node_ptr;

Answer_node_ptr answer_header_node = NULL; 

NodePtr headerNode = NULL; // Global variable for the head of the list 
NodePtr point = NULL; // Global variable for pointing current visited Node in the list. 

sem_t unused ; // unused thread counts 
sem_t inused ; // inuse thread counts 
pthread_mutex_t subtasks_lock = PTHREAD_MUTEX_INITIALIZER ; // to prevent multiple threads from accessing headerNode at the same time
pthread_mutex_t header_lock = PTHREAD_MUTEX_INITIALIZER ; 
pthread_mutex_t answer_lock = PTHREAD_MUTEX_INITIALIZER ; 

clock_t start_time, end_time;
double execution_time;


// this function should be called in sync
Answer_node_ptr find_header(char* path){
    if(answer_header_node == NULL) return NULL;

    Answer_node_ptr curr = answer_header_node;
    while(curr != NULL){
        if(strcmp(curr->path, path) == 0){
            return curr;
        }
        //printf("%s\n", curr->header_next->path);
        curr = curr->header_next;
    }
    return NULL; // which means the path has not been added
}

Answer_node_ptr insert_new_header(char* path){
    Answer_node_ptr new_header_node;
    if((new_header_node = find_header(path)) != NULL) {
        return new_header_node;
    }
    new_header_node = (Answer_node_ptr)malloc(sizeof(Answer_node));

    strcpy(new_header_node->path, path);
    new_header_node->header_next = answer_header_node;
    new_header_node->node_next = NULL;
    answer_header_node = new_header_node;
    return new_header_node;
}

void insert_new_node(Answer_node_ptr target_header, char* path){
    Answer_node_ptr new_node = (Answer_node_ptr)malloc(sizeof(Answer_node));
    Answer_node_ptr temp = target_header->node_next;
    strcpy(new_node->path, path);
    new_node->node_next = temp;
    new_node->header_next = NULL;
    target_header->node_next = new_node;
}

void update_header();
void add_node(SubtaskPtr node);

void insertNode(const char* path) {
    NodePtr newNode = (NodePtr)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    strcpy(newNode->path, path);
    newNode->prev = NULL;
    newNode->next = headerNode;
    if (headerNode != NULL) {
        headerNode->prev = newNode;
    }
    headerNode = newNode;
}

// if flag is 0 then delete node from the linked list but do not free the memory allocation
void deleteNode(NodePtr node, int flag) {
    if (node == NULL) {
        return;
    }

    if (node == headerNode) {
        headerNode = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    }

    if(flag){
        free(node);
    }
}

void freeList() {
    while (headerNode != NULL) {
        NodePtr temp = headerNode;
        headerNode = headerNode->next;
        free(temp);
    }
}

void searchFiles(const char* dirPath) {
    DIR* dir;
    struct dirent* entry;
    struct stat statBuf;
    char path[MAX_PATH_LENGTH];

    dir = opendir(dirPath);
    if (dir == NULL) {
        fprintf(stderr, "Error opening directory %s\n", dirPath);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, MAX_PATH_LENGTH, "%s/%s", dirPath, entry->d_name);
        if (lstat(path, &statBuf) == -1) {
            fprintf(stderr, "Error stating file %s\n", path);
            continue;
        }

        if (S_ISDIR(statBuf.st_mode)) {
            searchFiles(path); // Recursively search subdirectories
        } else if (S_ISREG(statBuf.st_mode)) {
            insertNode(path);
        }
    }

    closedir(dir);
}

int printList() {
    NodePtr current = headerNode;
    int count = 0;
    while (current != NULL) {
        printf("%s\n", current->path);
        count++;
        current = current->next;
    }
    return count;
}

// return code
// -1: error
//  0: different files
//  1: identical files
int compare_files_value(const char* filename1, const char* filename2) {
    FILE *file1, *file2;
    int byte1, byte2;

    file1 = fopen(filename1, "rb");
    file2 = fopen(filename2, "rb");

    if (file1 == NULL || file2 == NULL) {
        printf("Error opening files.\n");
        printf("filename1 :%s\n", filename1);
        printf("filename2 :%s\n", filename2);
        return -1;
    }

    // compare sizes
    struct stat st1;
    int result1 = stat(filename1, &st1);
    struct stat st2;
    int result2 = stat(filename2, &st2);

    if(result1 == -1 || result2 == -1)
        printf("error number= %d, error str= %s\n", errno, strerror(errno));
    if (result1 == 0 && result2 == 0 && (st1.st_size != st2.st_size)){
        // if different file sizes
        fclose(file1);
        fclose(file2);
        return 0;
    }

    // if same file sizes
    while (1)
    {
        byte1 = fgetc(file1);
        byte2 = fgetc(file2);
        if (byte1 == EOF)
            break;
        if (byte2 == EOF)
            break;

        if (byte1 != byte2)
        {
            fclose(file1);
            fclose(file2);
            return 0; // Files have different bytes
        }
    }

    // Check if both files have reached the end
    if ((byte1 == EOF) && (byte2 == EOF)) {
        // printf("[debug] Files have the same sequence of bytes\n");
        fclose(file1);
        fclose(file2);
        return 1;  // Files have the same sequence of bytes
    }

    // printf("[debug] Files have the diffrent sequence of bytes!\n");
    fclose(file1);
    fclose(file2);
    return 0;  // Files have different lengths
}

// this function should be called synchronically
void move_point(){
    point = point->next;
}

int evaluate(SubtaskPtr s){
   // todo: compare(s->path1, s->path2);
   int result = compare_files_value(s->path1->path, s->path2->path);
   switch(result){
    case 1:
        return result;
    case 0:
        return result;
    case -1:
        perror((char *) "error: failed to compare!");
        break;
    default:
        perror((char *) "undefined result");
        break;
   }
   return -1;
}

void update_header(){
    NodePtr newHeader = headerNode->next;
    pthread_mutex_lock(&header_lock);
        deleteNode(headerNode, 0);
        headerNode = newHeader;
    pthread_mutex_unlock(&header_lock);
}

// this function should be called synchronically
SubtaskPtr get_subtask(){
    SubtaskPtr s = (SubtaskPtr)malloc(sizeof(Subtask)); // malloc&free pair id # 2
	pthread_mutex_lock(&subtasks_lock) ;
        NodePtr p = point->next;

        if(p == 0x0) {
            update_header();
            point = headerNode;
            p = point->next;
        }
        
        move_point();
        
        pthread_mutex_lock(&header_lock);
            s->path1 = headerNode;
        pthread_mutex_unlock(&header_lock);

        s->path2 = p;
	pthread_mutex_unlock(&subtasks_lock) ;
    if(s->path2 == NULL) return NULL ;
    return s ;
}

void * worker (void * arg)
{
	SubtaskPtr s;
    pthread_t a = pthread_self();

	while ((s = get_subtask())) {
        // if s is null which means subtask bucket is empty, then call generate subtask function using original linked list.
        // if the original linked list is also empty, then escape the while loop. it means there is no job left.
		if(evaluate(s) == 1){
            pthread_mutex_lock(&answer_lock);
                add_node(s);
                deleteNode(s->path2, 0);
            pthread_mutex_unlock(&answer_lock);
        }
	}
}

void add_node(SubtaskPtr node){
    //printf("adding.. %s | %s\n", node->path1->path, node->path2->path);
    insert_new_node(insert_new_header(node->path1->path), node->path2->path);
    // free(node->path1);
    // free(node->path2);
    // free(node); // malloc&free pair id # 2
}
void printPaths() {
    Answer_node_ptr current = answer_header_node;

    printf("[\n");
    while (current != NULL) {
        printf("\t[\n\t\t%s\n", current->path);
        Answer_node_ptr node = current->node_next;
        while (node != NULL) {
            printf("\t\t%s,\n", node->path);
            node = node->node_next;
        }
        current = current->header_next;
        printf("\t],\n");
    }
    printf("]\n");
}
void signalHandler(int sig) {
    if (sig == SIGINT) {
        printPaths();

        end_time = clock();

        // 실행 시간 계산
        execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

        // 실행 시간 출력
        printf("실행 시간: %.2f초\n", execution_time);
        
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    int nthread ;
    char *output_file_path = NULL;
    char *init_directory = NULL;

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        int arglen = strlen(arg); 
        int start_index = 3;

        printf("Option %d: %s\n", i, arg);

        if(arg[1] == 't'){
            nthread = atoi(&arg[3]);
        }else if(arg[1] == 'm'){
            ignore_size = atoi(&arg[3]);
        }else if(arg[1] == 'o'){
            int substring_len = arglen - start_index;
            output_file_path = (char *)malloc((substring_len + 1) * sizeof(char));
            strncpy(output_file_path, arg + start_index, substring_len);
            output_file_path[substring_len] = '\0';
            printf("output_file_path:%s\n", output_file_path);
        }else{
            init_directory = (char *)malloc((arglen + 1) * sizeof(char));
            strncpy(init_directory, arg , arglen);
            init_directory[arglen] = '\0';
            printf("init_directory:%s\n", init_directory);
            DIR *dp = opendir(init_directory);
            if (dp == NULL)
            {
                printf("Error opening directory %s\n", init_directory);
                exit(1);
            }
            closedir(dp);
        }
    }
    if(nthread == -1){
        printf("number of thread must be inputted\n");
        exit(1);
    }
    if(init_directory == NULL){
        printf("Init directory must be inputted\n");
        exit(1);
    }
    
    pthread_t threads[nthread] ;

    signal(SIGINT, signalHandler);

    start_time = clock();

    searchFiles(init_directory);
    point = headerNode;

    printf("Found files:\n");
    int nodeCount = printList();
    printf("Number of files: %d\n", nodeCount);

    for (int i = 0 ; i < nthread ; i++) {
		pthread_create(&(threads[i]), NULL, worker, NULL) ;
	}

    for (int i = 0 ; i < nthread ; i++) {
		pthread_join((threads[i]), NULL) ;
	}

    end_time = clock();

    // 실행 시간 계산
    execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printPaths();

    // 실행 시간 출력
    printf("실행 시간: %.2f초\n", execution_time);



    // freeList(); -> list는 어짜피 비어있을 거임.. 위에서 Adjacency_linked_list_header만들면서 각각 free 해줘야 함.
    return 0;
}

