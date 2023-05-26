#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define SITENUM 100


int daysformonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; //days for each months
char* months[12] = {"Jan","Feb","March","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
char* get_date(int days){
    char* date;
    int month=0;
    for(int i=0;i<12;i++){
        if(days>daysformonth[i]){
            month++;
            days -= daysformonth[i];
        }
        else break;
    }

    date = (char*)malloc(sizeof(char) * 10);
    
    // Format the date string
    snprintf(date, 10, "%s %d%s", months[month], days, (days > 3 && days < 21) ? "th" : (days % 10 == 1) ? "st" : (days % 10 == 2) ? "nd" : (days % 10 == 3) ? "rd" : "th");
    
    return date;
}

typedef struct Node{
    int val;
    int color;//0 Black, 1 Red
    struct Node* parent;
    struct Node* left;
    struct Node* right;    
}Node;

typedef struct RBtree{
    Node* root;
    Node* NIL;
}RBtree;

RBtree* generateRBtree(){
    RBtree *T = (RBtree*)malloc(sizeof(RBtree));

    Node* NILnode = (Node*)malloc(sizeof(Node));
    NILnode->parent = NULL;
    NILnode->left = NULL;
    NILnode->right = NULL;
    NILnode->color = 0; //NIL 노드 생성

    T->NIL = NILnode; 
    T->root = T->NIL; //노드 0개일 때 root는 NIL임
    return T;
}

void printBST(RBtree* T,Node* root,int space)
{
    if (root == T->NIL) return;

    space += 5;
    printBST(T,root->right, space);
    printf("\n");
    for (int i = 5; i < space; i++) printf(" ");
    if(root->color==0)
        printf("%dB \n", root->val);
    else
        printf("%dR \n",root->val);
    printBST(T,root->left, space);
}

void printinorder(RBtree* T, Node* node){
    if(node == T->NIL) return;

    printinorder(T,node->left);
    if(node->color==0) printf("%dB",node->val);
    else printf("%dR ",node->val);
    printinorder(T,node->right);
    return;
}

void leftRotate(RBtree* T, Node* node){
    Node* y = node->right;
    node->right = y->left;

    if(y->left!=T->NIL)
        y->left->parent = node;
    y->parent = node->parent;
    if(node->parent == T->NIL) T->root = y;
    else if(node->parent->left == node) node->parent->left = y;
    else node->parent->right = y;

    y->left = node;
    node->parent = y;
    return;
}

void rightRotate(RBtree* T, Node* node){
    Node* y = node->left;
    node->left = y->right;

    if(y->right!=T->NIL)
        y->right->parent = node;
    y->parent = node->parent;
    if(node->parent == T->NIL) T->root = y;
    else if(node->parent->left == node) node->parent->left = y;
    else node->parent->right = y;

    y->right = node;
    node->parent = y;
    return;
}

Node* BSTsearch(RBtree* T,int val){ //해당 노드 또는 삽입 위치 노드 리턴
    Node* x = T->root;
    if(x==T->NIL) return T->NIL;

    while(x->val != val){
        if(x->val > val && x->left != T->NIL)
            x = x->left;
        else if(x->val < val && x->right != T->NIL)
            x = x->right;
        else break;
    }
    return x;
}

void RBInsertFixup(RBtree* T,Node* node){
    Node* y; //uncle of node
    while(node->parent->color == 1){
        if(node->parent == node->parent->parent->left){
            y = node->parent->parent->right;
            if(y->color == 1){ //Red
                node->parent->color = 0;
                y->color = 0;
                node->parent->parent->color = 1;
                node = node->parent->parent;
            }
            else{ //uncle is Black, node is right child 
                if(node == node->parent->right){ //case 2
                    node = node->parent;
                    leftRotate(T,node);
                }
                node->parent->color = 0;
                node->parent->parent->color = 1;
                rightRotate(T,node->parent->parent);
            }
        }
        else{ //node->parent == node->parent->parent->right
            y = node->parent->parent->left;
            if(y->color == 1){ //Red
                node->parent->color = 0;
                y->color = 0;
                node->parent->parent->color = 1;
                node = node->parent->parent;
            }
            else{ //uncle is Black, node is left child 
                if(node == node->parent->left){
                    node = node->parent;
                    rightRotate(T,node);
                }
                node->parent->color = 0;
                node->parent->parent->color = 1;
                leftRotate(T,node->parent->parent);
            }
        }
    }
    T->root->color = 0;
    return;
}

void RBDeleteFixup(RBtree* T,Node* node){
    Node* w; //sibling of node
    while(node != T->root && node->color == 0){
        if(node == node->parent->left){ 
            w = node->parent->right;
            
            if(w->color == 1){ //case 1 ->case2,3,4
                w->color = 0;
                node->parent->color = 1;
                leftRotate(T,node->parent);
                w = node->parent->right;
            }
            if(w->left->color == 0 && w->right->color == 0){ //case2
                w->color = 1;
                node = node->parent;
            }
            else{
                if(w->right->color == 0){ //case 3
                    w->left->color = 0;
                    w->color = 1;
                    rightRotate(T,w);
                    w = node->parent->right;
                }
                w->color = node->parent->color;
                node->parent->color = 0;
                w->right->color = 0;
                leftRotate(T,node->parent);
                node = T->root;
            }
        }
        else{ //node == node->parent->right
            w = node->parent->left;
            if(w->color == 1){ //case 1 ->case2,3,4
                w->color = 0;
                node->parent->color = 1;
                rightRotate(T,node->parent);
                w = node->parent->left;
            }
            if(w->left->color == 0 && w->right->color == 0){ //case2
                w->color = 1;
                node = node->parent;
            }
            else{
                if(w->left->color == 0){ //case 3
                    w->right->color = 0;
                    w->color = 1;
                    leftRotate(T,w);
                    w = node->parent->left;
                }
                w->color = node->parent->color; //case 4
                node->parent->color = 0;
                w->left->color = 0;
                rightRotate(T,node->parent);
                node = T->root;
            }
        }
    }
    node->color = 0;
    return;
}

void RBinsert(RBtree* T, int val){
    Node* parent = BSTsearch(T,val);
    if(parent->val == val) return; // 중복 값 무시
    Node* target = (Node*)malloc(sizeof(Node));
    target->left = T->NIL;
    target->right = T->NIL;
    target->val = val;
    target->color = 1; //initial color: red

    if(parent == T->NIL){
        T->root = target;
        target->parent = T->NIL;
    }

    if(parent->val < val)
        parent->right = target;
    else
        parent->left = target;    
    target->parent = parent;

    RBInsertFixup(T,target);
    return;
}

Node* successor(RBtree* T, Node* target){ //return val otherwise return successor(val)
    Node* x;
    if(target->right != T->NIL){
        x = target->right;
        while(x->left != T->NIL){
            x = x->left;
        }
        return x;
    }
    else{
        x = target->parent;
        while(x != T->NIL && target == x->right){
            target = x;
            x = x->parent;
        }
        return x; //NIL if successor DNE
    }
}

Node* BSTsearchprime(RBtree* T, int val){ //return val otherwise return successor(val)
    Node* target = BSTsearch(T,val);
    if(target->val >= val) return target;
    else return successor(T,target);
    
}

void RBdelete(RBtree* T,int val){
    Node* target = BSTsearch(T,val);
    if(target == T->NIL) return;
    Node* y; //node to splice out
    Node* x; //child of y
    
    if(target->left == T->NIL || target->right == T->NIL) //case 1,2: delete target
        y = target;
    else y = successor(T,target); //case 3: delete successor

    if(y->left != T->NIL) //set x to the child of y
        x = y->left;
    else
        x = y->right;

    x->parent = y->parent;

    if(y->parent == T->NIL){ // if y is a root node
        T->root = x;
    }
    else{
        if(y == y->parent->left)
            y->parent->left = x;
        else y->parent->right = x;
    }
    if(target != y)
        target->val = y->val;
    
    if(y->color == 0)
        RBDeleteFixup(T,x);
    free(y);
    return;
}

/*functions and structures for RBtree*/

typedef struct SITE
{
    int id; // # of site
    int duration; // time it takes for tour
    int cost; // cost for the tour 
    RBtree* hotel; //root node for hotel RBtree
}SITE;
typedef struct HOTEL
{
    int id;
    int cost; //40000won ~ 60000won /day
}HOTEL;

typedef struct PATH
{
    short connected; //1 if there exists path.   
    short type; // 0: bus, 1: metro, 2: train, 3: taxi  Order matters for the cost.
    int cost; // cost of transportation
}PATH;

SITE* SITES; //global graph variable for sites and paths.
PATH** PATHS;

void initialize_SITE(){
    SITES = (SITE*)malloc(sizeof(SITE)*SITENUM);
    /*generate 100 sites*/
    for(int i=0;i<SITENUM;i++){
        SITES[i].id = i;
        SITES[i].duration = (rand()%3+1); // tour takes minimum 1 day to maximum 3 days.
        SITES[i].cost = 50000 + (rand()%20+20)*1000; // 50000won for default, and add random weight proportion to duration.
        //additional cost is 20000won ~ 40000won /day.
        SITES[i].hotel = generateRBtree(); //create hotel RBtree.
    }
    /*set duration, cost for every sites*/
    return;
}

void initialize_Hotel(RBtree* root){
    

    return;
}

void initialize_PATH(){ //make a graph containing 100 sites and 300 paths.
    
    int i,j;
    PATHS = (PATH**)calloc(SITENUM,sizeof(PATH*));
    for(i=0;i<SITENUM;i++){
        PATHS[i] = (PATH*)calloc(SITENUM,sizeof(SITE));
    }
    /*generate and initialize matrix to 0*/
    
    int** IndexMap = (int**)calloc(10,sizeof(int*)); 
    for(i=0;i<10;i++){
        IndexMap[i] = (int*)calloc(10,sizeof(int));
    }
    int tempArr[SITENUM];
    for(i=0;i<SITENUM;i++){
        tempArr[i] = rand()%100;
        for(j=0;j<i;j++){
            if(tempArr[i] == tempArr[j]) i--;
        }
    }
    for(i=0;i<10;i++){
        for(j=0;j<10;j++){
            IndexMap[i][j] = tempArr[10*i+j];

        }
    }
    /*set IndexMap for random SITE and PATH making*/
    /*now, generate (almost) random 300 paths*/
    
    /*1. Make 100 paths which connect each (i)th row of IndexMap linearly */
    for(i=0;i<10;i++){
        for(j=0;j<10;j++){
            PATHS[IndexMap[i][j]][IndexMap[i][(j+1)%10]].connected = 1;
            PATHS[IndexMap[i][(j+1)%10]][IndexMap[i][j]].connected = 1;
        }
    }
    /*2. Between each (i)th and (i+1) row, make random 20 paths. */
    int a,b;
    for(i=0;i<10;i++){
        for(j=0;j<20;j++){
            a = rand()%10;
            b = rand()%10;
            if(PATHS[IndexMap[i%10][a]][IndexMap[(i+1)%10][b]].connected == 1){
                j--;
            }
            PATHS[IndexMap[i%10][a]][IndexMap[(i+1)%10][b]].connected = 1;
            PATHS[IndexMap[(i+1)%10][b]][IndexMap[i%10][a]].connected = 1;
        }
    }
    return;
}

void printpath(){
    for(int i=0;i<10;i++){
        for(int j=0;j<10;j++){
            printf("%d ",PATHS[i][j].connected);
        }
        printf("\n");
    }

    return;
}

SITE* find_next_dest(SITE* depart,int budget,int leftDays){ 
    //from site(depart), find candidates for next tour site considering leftover budget and dates.
    int i;
    for(i=0;i<SITENUM;i++){
        if(PATHS[depart->id][i].connected == 1){

        }
    }



    return Next;
}






int main(void){
    srand((unsigned int)time(NULL));
    // int* tourPath; //sequence of sites during tour.
    // HOTEL* tourHotel; //reserved hotel for each days. 
    initialize_SITE(); //initialize sites and paths.
    initialize_PATH();
    
    return 0;
}