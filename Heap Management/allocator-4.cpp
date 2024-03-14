//yigit kaan 29154
#include <iostream>
#include <pthread.h>

using namespace std;

struct node {
    int id;
    int size;
    int index;
    node* next_ptr;
    node(int i, int s, int idx, node* n): id(i), size(s), index(idx), next_ptr(n) {}
};

class HeapManager{
private:
    node* head_ptr;
    pthread_mutex_t lock_t;
public:
    HeapManager();
    int initHeap(int size);
    int myMalloc(int ID, int size);
    int myFree(int ID, int index);
    void print();
};

HeapManager::HeapManager(){
    head_ptr = nullptr;
    pthread_mutex_init(&lock_t, nullptr);
}

int HeapManager::initHeap(int size) {
    head_ptr = new node(-1, size, 0, NULL);
    cout << "Memory initialized" << endl;
    this->print();
    return 1;
}

int HeapManager::myMalloc(int ID, int size){
    pthread_mutex_lock(&lock_t);
    node* curr = head_ptr;
    node* prev = nullptr;
    int ret = -1;
    
    while(curr){
        if(curr->id == -1 && curr->size > size){
            ret = curr->index;
            node* new_ = new node(ID, size, curr->index, curr);
            curr->size = curr->size - size;
            curr->index = curr->index + size;
            
            if(!prev){
                this->head_ptr = new_;
            }else{
                prev->next_ptr = new_;
            }
            
            cout << "Allocated for thread " << ID << endl;
        
            break;
        }
        else if(curr->id == -1 && curr->size == size){
            ret = curr->index;
            node* new_ = new node(ID, size, curr->index, curr->next_ptr);
            node* del = curr;
            
            if(!prev){
                this->head_ptr = new_;
            }else{
                prev->next_ptr = new_;
            }
            delete del;
            
            cout << "Allocated for thread " << ID << endl;
            
            break;
        }
        prev = curr;
        curr = curr->next_ptr;
    }
    
    if(ret == -1){
        cout << "Cannot allocate, requested size " << size << " for thread " << ID << " is bigger than remaining size" << endl;
    }
    
    this->print();
    pthread_mutex_unlock(&lock_t);
    
    return ret;
}

int HeapManager::myFree(int ID, int index){
    pthread_mutex_lock(&lock_t);
    node* curr = head_ptr;
    node* prev = nullptr;
    int ret = -1;
    
    while(curr){
        if(curr-> id == ID && curr->index == index){
            curr->id = -1;
            
            if(curr->next_ptr){
                if(curr->next_ptr->id == -1){
                    node* del = curr->next_ptr;
                    curr->size = curr->size + curr->next_ptr->size;
                    curr->next_ptr = curr->next_ptr->next_ptr;
                    delete del;
                }
            }
            
            if(prev){
                if(prev->id == -1){
                    node* del = curr;
                    prev->size = prev->size + curr->size;
                    prev->next_ptr = curr->next_ptr;
                    delete del;
                }
            }
            
            cout << "Freed for thread " << ID << endl;
            ret = 1;
            break;
        }
        prev = curr;
        curr = curr->next_ptr;
    }
    
    if(ret == -1){
        cout << "Cannot deallocate, there is no thread " << ID << " at index of " << index << endl;
    }
    
    this->print();
    pthread_mutex_unlock(&lock_t);
    return ret;
}

void HeapManager::print() {
    node* temp = head_ptr;
    while(temp){
        cout << "[" << to_string(temp->id) << "][" << to_string(temp->size) << "][" << to_string(temp->index) << "]";
        if(temp->next_ptr){
            cout << "---";
        }
        temp = temp->next_ptr;
    }
    cout << endl;
}
