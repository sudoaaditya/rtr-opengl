#include<iostream>

class Node {

    public:
        int m_Data;
        class Node *m_Next;
};

typedef class Node Node;

Node *head = nullptr, *travel = nullptr;

class LinkedList {

    private:
        int m_nodeCnt;

    public:

        LinkedList(int);
        ~LinkedList();

        void CreateNode();
        void Display();
        void InsertData(Node*&);
        void DeleteLast();
};

LinkedList::LinkedList(int cnt) {
    m_nodeCnt = cnt;
}

LinkedList::~LinkedList() {

    travel = head;
    std::cout<<"Called Destructor"<<std::endl;
    if(head == nullptr) {
        return;
    }
    while(head != nullptr) {

        DeleteLast();

    }
}

void LinkedList::CreateNode() {

    for(int i=0; i<m_nodeCnt;i++) {
        Node *newNode = new Node();

        InsertData(newNode);

        if(head == nullptr) {
            head = newNode;
        }
        else {
            travel = head;

            while(travel->m_Next != nullptr) {
                travel = travel->m_Next;
            }
            travel->m_Next = newNode;
        }
    }
}

void LinkedList::InsertData(Node* &pNode)  {

    std::cout<<"Enter data : ";
    std::cin>>pNode->m_Data;

    pNode->m_Next=nullptr;
}

void LinkedList::Display() {
    
    if(head == nullptr) {
            std::cout<<"Called Wrong"<<std::endl;
            return;
        }
    else {
        travel = head;

        while(travel->m_Next != nullptr) {
            std::cout<<"|"<<travel->m_Data<<"|->>";
            travel = travel->m_Next;
        }
        std::cout<<"|"<<travel->m_Data<<"|\n";
    }
}

void LinkedList::DeleteLast() {

    Node *temp;

    if(head == nullptr) {
        std::cout<<"Called Wrong"<<std::endl;
        return;
    }
    else {
        travel = head;

        while(travel->m_Next->m_Next != nullptr) {
            travel = travel->m_Next;
        }
        temp = travel->m_Next;
        travel->m_Next = nullptr;
        delete(temp);
    }
}


int main() {

    LinkedList *ll = new LinkedList(10);

    ll->CreateNode();

    ll->Display();

    delete(ll);

    return(0);
}
