/// This is a homework 1 in Algorithm Analysis course
/// Application Contributer: Shinhoo Kim (only)
/// Student Id: 21900136
/// 
/// Reference List Below
/// Stackoverflow: https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library by https://stackoverflow.com/users/4534873/happy-sisyphus
///		-to generate uuid for each queue and to share the id with its elements to allow the contents in the element to be accessed through the queue class but not by the other accesses.

#include <iostream>
#include <string>
#include <stdexcept>
#include <random>
#include <sstream>

namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}
class SizeIsZeroException : std::exception{
	public:
	const char * what () const throw ()
    {
    	return "Cannot find an element: Size is zero";
    }
};
class IdIsNot4DigitsException : std::exception{
	public:
	const char * what () const throw ()
    {
    	return "Id should be 4 digits numeric value";
    }
};
class IdIsNotNumericException : std::exception {
public:
	const char* what() const throw ()
	{
		return "Id should be numeric!";
	}
};
class InvalidIndexException : std::exception{
	public:
	const char * what (int targetIndex) const throw ()
    {
    	return "Cannot find an element corresponding index : " + targetIndex;
    }
};
class PermissionDeniedToUpdatePriorityException : std::exception{
	public:
	const char * what () const throw ()
    {
    	return "Permission denied to update elemet's priority";
    }
};
enum School {
	Handong, Doodong, Sedong, NotDefined
};
enum Menu {
	Insert, Delete, Decrease, PrintOut, Quit, Unknown
};
enum CapacityState {
	ShouldIncrease, ShouldDecrease, JustFine
};
enum AppState {
	Running, Zombie, Exit
};
inline const Menu getMenuFromChar(char c) {
	switch (c) {
		case 'I': return Menu::Insert;
		case 'D': return Menu::Delete;
		case 'C': return Menu::Decrease;
		case 'P': return Menu::PrintOut;
		case 'Q': return Menu::Quit;
		default: return Menu::Unknown;
	}
}
inline const char* enumToString(School school) {
	switch (school) {
		case Handong: return "Handong";
		case Doodong: return "Doodong";
		case Sedong: return "Sedong";
		default: return "[Unknown School]";
	}
}
inline const std::string castIntToString(int intVal) {
	char charArr[16];
	char answer[16];
	int i = 0;
	while (intVal != 0) {
		charArr[i++] = (intVal % 10) + '0';
		intVal /= 10;
	}
	int j = 0;
	while (j < i) {
		answer[j] = charArr[i - 1 - j];
		j++;
	}
	answer[j] = '\0';
	return answer;
}
inline const int castStringToInt(std::string stringVal) {
	int answer = 0;
	for (char c : stringVal) {
		if (c >= '0' && c <= '9') {
			answer *= 10;
			answer += ((int)c) - '0';
		} 
		else {
			throw IdIsNotNumericException();
		}
	}
	return answer;
}
inline void toLowercase(std::string& input){
	int index = 0;
	while(input[index] != '\0'){
		if(input[index] >= 'A' && input[index] <= 'Z'){
			input[index] += 32;
		}
		index++;
	}
}
inline const School getSchoolFromString(std::string schoolInString) {
	toLowercase(schoolInString);
	if (schoolInString.compare("handong") == 0) {
		return School::Handong;
	}
	if (schoolInString.compare("doodong") == 0) {
		return School::Doodong;
	}
	if (schoolInString.compare("sedong") == 0) {
		return School::Sedong;
	}
	return School::NotDefined;
}

class Element {
	std::string queueId;
	std::string name;
	int  id;
	std::string idInStringType;
	School school;
	size_t idLength=0;

	int getLengthOfInt(int intValue) {
		int num = intValue;
		int len = 0;
		while (num != 0) {
			len++;
			num /= 10;
		}
		return len;
	}
	void validateQueueId(std::string queueId){
		if(this->queueId != queueId) throw PermissionDeniedToUpdatePriorityException();
		
	}
	void validateElementId(int testId){
		if(castIntToString(testId).length() != 4) throw IdIsNot4DigitsException();
	}
	
public:
	Element(){}
	Element(const std::string name, int id, School school, std::string queueId) {
		validateElementId(id);
		this->queueId = queueId;
		this->name = name;
		this->id = id;
		this->idInStringType = castIntToString(id);
		this->idLength = this->idInStringType.length();
		this->school = school;
	}

	int getId() {
		return this->id;
	}
	void setId(std::string queueId, int newVal) {
		validateQueueId(queueId);
		validateElementId(newVal);
		this->id = newVal;
		this->idInStringType = castIntToString(id);
		this->idLength = this->idInStringType.length();
	}
	
	const std::string toString() {
		return "[" + this->name + ", " + this->idInStringType + ", " + enumToString(this->school) + "]";
	}
};

class AppString {
public:
	const std::string menuHeader = "*********** MENU ****************";
	const std::string insertMenu = "I : Insert new element into queue";
	const std::string deleteMenu = "D : Delete element with smallest key from queue";
	const std::string decreaseMenu = "C : Decrease key of element in queue";
	const std::string printOutMenu = "P : Print out all elements in queue";
	const std::string quitMenu = "Q : Quit";
	const std::string choosMenu = "Choose menu : ";
	const std::string enterNameOfElement = "Enter name of element: ";
	const std::string enterIdOfElement = "Enter id of element: ";
	const std::string enterSchoolOfElement = "Enter school of element: ";
	const std::string enterIndexOfElement = "Enter index of element: ";
	const std::string exit = "Thank you. Bye!";
	const std::string newElement(Element element) {
		return "New element " + element.toString() + " is inserted";
	}
	const std::string deleteElement(Element element) {
		return element.toString() + " is deleted";
	}
	const std::string increasingWarning(){
		return "Warning: This decrease function is supposed to decrease the id key. But the id key is increased";
	}
};

class MinPriorityQueue {
	std::string queueId = uuid::generate_uuid_v4();
	Element nodes[30];
	int size;
	int capacity;

	void swap(int i, int j) {
		Element temp = nodes[i];
		nodes[i] = nodes[j];
		nodes[j] = temp;
	}

	bool swim(int targetNodeIndex) {
		Element child = nodes[targetNodeIndex];
		int parentIndex = targetNodeIndex / 2;
		Element parent = nodes[parentIndex];
		if (child.getId() < parent.getId()) {
			swap(targetNodeIndex, parentIndex);
			return true;
		}
		return false;
	}

	int sink(int targetNodeIndex){
		int leftChildIndex = targetNodeIndex * 2;
		if(leftChildIndex > size) return 0;
		int rightChildIndex = targetNodeIndex * 2 + 1;
		
		int currPriority = nodes[targetNodeIndex].getId(), 
			leftPriority = nodes[leftChildIndex].getId(), 
			rightPriority = rightChildIndex > size ? -1 : nodes[rightChildIndex].getId();
		
		int minPriorityIndex = targetNodeIndex;

		if(rightPriority != -1 && rightPriority < leftPriority){
			if(rightPriority < currPriority){
				minPriorityIndex = rightChildIndex;
			}
		} else{
			if(leftPriority < currPriority){
				minPriorityIndex = leftChildIndex;
			}
		}
		if(minPriorityIndex == targetNodeIndex){
			return 0; //no changes
		}
		swap(targetNodeIndex, minPriorityIndex);
		return minPriorityIndex;
	}

	void minHeapifySwiming(int targetInd = -1) {
		int start = targetInd != -1 ? targetInd : size;
		while (start > 1) {
			if (!swim(start)) {
				break; //if there is no change between a child and its parent, then you assume the tree is heapified.
			}
			start /= 2; //go to the parent element
		}
	}
	void minHeapifySinking(int targetInd = -1) {
		int start = targetInd != -1 ? targetInd : 1;
        while (start <= size) {
			int nextIndex = sink(start);
            if(nextIndex == 0){
				break;
			}
			start = nextIndex;
        }
	}

	CapacityState _checkCapacityState() {
		int lowBoundary = capacity / 4;
		if (size == capacity) {
			return CapacityState::ShouldIncrease;
		}
		else if (lowBoundary != 0 && size <= lowBoundary) {
			return CapacityState::ShouldDecrease;
		}
		else {
			return CapacityState::JustFine;
		}
	}

	void maybeChangeCapacity() {
		switch (_checkCapacityState()) {
		case CapacityState::JustFine:
			return;
		case CapacityState::ShouldDecrease:
			return;
		case CapacityState::ShouldIncrease:
			return;
		}
	}
	void validateTargetElementIndex(int indexToValidate){
		if(indexToValidate < 1 || indexToValidate > size) throw InvalidIndexException();
	}

public:
	MinPriorityQueue() {
		size = 0;
		capacity = 1;
	}
	std::string getQueueId(){
		return this->queueId;
	}
	void grow(Element nodeToAdd) {
		maybeChangeCapacity(); 
		nodes[++size] = nodeToAdd;
		minHeapifySwiming();
	}
	Element top(){
		if (size == 0) throw SizeIsZeroException();
		return nodes[1];
	}


	Element trim() {
		if (size == 0) throw SizeIsZeroException();
		Element targetElementToDelete = nodes[1];
		swap(1, size);
		--(this->size);
		maybeChangeCapacity();
		minHeapifySinking();
		return targetElementToDelete;
	}

	Element* getNodes() {
		return nodes;
	}
	
	void modifyElementPriority(int targetIndex, int newIdVal){
		if (size == 0) throw SizeIsZeroException();
		validateTargetElementIndex(targetIndex);
		try{
			int oldId = nodes[targetIndex].getId();
			if(oldId == newIdVal) return;
			nodes[targetIndex].setId(queueId, newIdVal);
			if(oldId > newIdVal){
				minHeapifySwiming(targetIndex);
			} else{
				std::cout << AppString().increasingWarning() << std::endl;
				minHeapifySinking(targetIndex);
			}
		} catch(PermissionDeniedToUpdatePriorityException e){
			std::cout << e.what() << std::endl << std::endl;
		}
	}

	void printAll() {
		for (int i = 1; i <= size; i++) {
			std::cout << nodes[i].toString();
		}
		std::cout << "\n\n";
	}

};

class AppService {
	
	AppString appString = AppString();
	Menu selectedMenu;
	AppState appState;
	MinPriorityQueue minPriorityQueue;

	void INSERT(MinPriorityQueue& S, Element x){
		S.grow(x);
	}
	Element MINIMUM(MinPriorityQueue& S){
		return S.top();
	}
	void EXTRACT_MIN(MinPriorityQueue& S){
		Element deletedElemet = S.trim();
		std::cout << appString.deleteElement(deletedElemet) << std::endl;
		std::cout << std::endl;
	}
	void DECREASE_KEY(MinPriorityQueue& S, int x, int k){
		S.modifyElementPriority(x, k);
	}

	void _dispose() {
		selectedMenu = Menu::Unknown;
		appState = AppState::Exit;
	}
	void printMenuOnConsole() {
		std::cout << appString.menuHeader << std::endl;
		std::cout << appString.insertMenu << std::endl;
		std::cout << appString.deleteMenu << std::endl;
		std::cout << appString.decreaseMenu << std::endl;
		std::cout << appString.printOutMenu << std::endl;
		std::cout << appString.quitMenu << std::endl;
		std::cout << std::endl;
		std::cout << appString.choosMenu;
	}
	void setMenu() {
		std::string input;
		std::cin >> input;
		std::cin.ignore();
		selectedMenu = getMenuFromChar(input[0]);
	}

	Element getNewElement() {
		std::string name;
		std::string idInString;
		int  id;
		std::string schoolInString;
		School school;
		
		std::cout << appString.enterNameOfElement; getline(std::cin, name);
		std::cout << appString.enterIdOfElement; std::cin >> id;
		std::cout << appString.enterSchoolOfElement; std::cin >> schoolInString;
		school = getSchoolFromString(schoolInString);
		return Element(name, id, school, minPriorityQueue.getQueueId());
	}
	
	void insertAction() {
		try {
			const Element newElement = getNewElement();
			INSERT(minPriorityQueue, newElement);
			std::cout << appString.newElement(newElement) << std::endl;
			std::cout << std::endl;
		}
		catch (IdIsNotNumericException e) {
			std::cout << e.what() << std::endl << std::endl;
		}
		catch (IdIsNot4DigitsException e) {
			std::cout << e.what() << std::endl << std::endl;
		}
	}

	void printAll() {
		minPriorityQueue.printAll();
	}

	void deleteAction() {
		try{
			EXTRACT_MIN(minPriorityQueue);
		} catch(SizeIsZeroException e){
			std::cout << e.what() << std::endl << std::endl;
		}
	}
	
	void decreaseAction(){
		std::string targetIndexInString;
		std::string newIdValueInString;
		int targetIndex;
		int newIdValue;
		try{
			std::cout << appString.enterIndexOfElement;
			std::cin >> targetIndexInString; targetIndex = castStringToInt(targetIndexInString);
			std::cout << appString.enterIdOfElement;
			std::cin >> newIdValueInString; newIdValue = castStringToInt(newIdValueInString);
			DECREASE_KEY(minPriorityQueue, targetIndex, newIdValue);
		} catch(SizeIsZeroException e){
			std::cout << e.what() << std::endl << std::endl;
		} catch(InvalidIndexException e){
			std::cout << e.what(targetIndex) << std::endl << std::endl;
		} catch (IdIsNot4DigitsException e) {
			std::cout << e.what() << std::endl << std::endl;
		}
	}

	void printSelectedMenuOnConsole() {
		switch (selectedMenu) {
		case Menu::Insert:
			insertAction();
			break;
		case Menu::Delete:
			deleteAction();
			break;
		case Menu::Decrease:
			decreaseAction();
			break;
		case Menu::PrintOut:
			printAll();
			break;
		case Menu::Quit:
			appState = AppState::Zombie;
			break;
		}
	}
	
public:
	AppService() {
		this->selectedMenu = Menu::Unknown;
		this->appState = AppState::Running;
		this->minPriorityQueue = MinPriorityQueue();
	}
	AppState getAppState() {
		return appState;
	}
	void terminateAppService() {
		std::cout << appString.exit << std::endl;
		_dispose();
	}
	
	void run() {
		printMenuOnConsole();
		setMenu();
		printSelectedMenuOnConsole();
	}
};

int main() {
	AppService appService = AppService();

	while (appService.getAppState() != AppState::Exit) {
		switch (appService.getAppState()) {
			case AppState::Running:
				appService.run();
				break;
			case AppState::Zombie:
				appService.terminateAppService();
				break;
		}
	}

	return 0;
}