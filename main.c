#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SITENUM 100

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Consume and discard characters
    }
    return;
}

const int daysformonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; //days for each months
const char* months[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char* transportation[4] = {"Metro","Train","Cruise","Taxi"};
char* get_StrDate(int days){
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

char* get_trans(int idx){
    char* trans;
    trans = (char*)malloc(sizeof(char) * 8);
    
    // Format the date string
    snprintf(trans, 8, "%s",transportation[idx]);
    
    return trans;
}

int get_IntDate(int month,int day){
    int Date=0;
    for(int i=0;i<month-1;i++){
        Date += daysformonth[i];
    }
    return Date+day;
}
//this data is to keep all the previous sites

typedef struct Hotel //will be stored to RBtree.
{
    int id;
    int color;
    int cost; //40000won ~ 60000won /day
    struct Hotel* left;
    struct Hotel* right;
    struct Hotel* parent;
}Hotel;

typedef struct RBHotel{
    Hotel* root;
    Hotel* NIL;
}RBHotel;

typedef struct SITE{
    int id; // # of site
    int duration; // time it takes for tour
    int cost; // cost for the tour 
    RBHotel* Hotel; //root node for Hotel RBtree
    int minHotelcost; //minimum Hotel cost
}SITE;

typedef struct PATH{
    short connected; //1 if there exists path.   
    short type; // 0: public transportation, 1: train, 2. cruise, 3: taxi. order matters for the cost.
    int cost; // cost of transportation
}PATH;

typedef struct TOUR{ //linked list to store reservated sites
    SITE* cursite; //also contains hotel information
    Hotel* hotel;
    PATH* path;
    struct TOUR* nextsite; //implicitly contains transportation(path)
}TOUR;


int searchHistory(TOUR* head,int id){ //search if SITE[id] has already been reserved.
    while(head != NULL){
        if(head->cursite->id == id) return 1; //already reserved.
        head = head->nextsite;
    }
    return 0;
}

typedef struct RESV{ //reservation type which contains customer id and tour information.
    // will be stored to RBtree.
    int id; //customer id
    int color;//0 Black, 1 Red
    int startdate;
    TOUR* tour; 
    SITE* lastsite;
    int period;
    int init_budget;
    int leftdays; //
    int budget; // these two variables will constantly change as user proceed in reservation.

    struct RESV*  parent;
    struct RESV*  left;
    struct RESV*  right;    
}RESV; 

typedef struct RBRESV{
    RESV* root;
    RESV* NIL;
}RBRESV;

//global variable for Reservation storing RBtree.
RBRESV* RBresv;

//global graph variable for sites and paths.
SITE* SITES; 
PATH** PATHS;

void insert_site(RESV* resv,int sitenum,Hotel* hotel){ //insert new site to TOUR linked list.
    TOUR* newSite = (TOUR*)malloc(sizeof(TOUR));
    /*create new TOUR node*/
    newSite->cursite = &SITES[sitenum];
    newSite->nextsite = NULL;
    newSite->hotel = hotel;
    /*If it is the first destination*/
    if(resv->tour == NULL){
        resv->tour = newSite;
        resv->budget -= newSite->cursite->cost + newSite->hotel->cost*newSite->cursite->duration;
        resv->leftdays -= newSite->cursite->duration;
        resv->lastsite = newSite->cursite;
        newSite->path = NULL;
        return;
    }
    /*If it is not the first destination*/
    else{
        TOUR* iter = resv->tour;
        while(iter->nextsite != NULL){
            iter = iter->nextsite;
        }
        iter->nextsite = newSite;
        /* budget -= tour cost, hotel cost, and transporation cost */
        /* remaining days -= site tour period */
        resv->budget -= newSite->cursite->cost + newSite->hotel->cost*newSite->cursite->duration 
                        + PATHS[resv->lastsite->id][sitenum].cost;
        resv->leftdays -= newSite->cursite->duration;
        newSite->path = &PATHS[resv->lastsite->id][sitenum];
        resv->lastsite = newSite->cursite;
    }
    return;
}

void inorderFilterRBHotel(RBHotel* rbhotel, Hotel* hotel,int budget,int duration,int* hotelnum,Hotel** hotellist){
    
    if(hotel == rbhotel->NIL) return;

    inorderFilterRBHotel(rbhotel,hotel->left,budget,duration,hotelnum,hotellist);
    if(hotel->cost*duration < budget){
        hotellist[++(*hotelnum)] = hotel;
    }
    inorderFilterRBHotel(rbhotel,hotel->right,budget,duration,hotelnum,hotellist);
}

RBHotel* generateRBHotel(){
    RBHotel *T = (RBHotel*)malloc(sizeof(RBHotel));

    Hotel* NILnode = (Hotel*)malloc(sizeof(Hotel));
    NILnode->parent = NULL;
    NILnode->left = NULL;
    NILnode->right = NULL;
    NILnode->color = 0; //NIL 노드 생성

    T->NIL = NILnode; 
    T->root = T->NIL; //노드 0개일 때 root는 NIL임
    return T;
}

void printBSTHotel(RBHotel* T,Hotel* root,int space)
{
    if (root == T->NIL) return;

    space += 5;
    printBSTHotel(T,root->right, space);
    printf("\n");
    for (int i = 5; i < space; i++) printf(" ");
    if(root->color==0)
        printf("%dB, %dwon \n", root->id,root->cost);
    else
        printf("%dR, %dwon \n", root->id,root->cost);
    printBSTHotel(T,root->left, space);
}

void leftRotateHotel(RBHotel* T, Hotel* node){
    Hotel* y = node->right;
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

void rightRotateHotel(RBHotel* T, Hotel* node){
    Hotel* y = node->left;
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

Hotel* BSTsearchbycostHotel(RBHotel* T,int cost){
    Hotel* x = T->root;
    if(x==T->NIL) return T->NIL;

    while(x->cost != cost){
        if(x->cost > cost && x->left != T->NIL)
            x = x->left;
        else if(x->cost < cost && x->right != T->NIL)
            x = x->right;
        else break;
    }
    return x;
}

void RBInsertFixupHotel(RBHotel* T,Hotel* node){
    Hotel* y; //uncle of node
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
                    leftRotateHotel(T,node);
                }
                node->parent->color = 0;
                node->parent->parent->color = 1;
                rightRotateHotel(T,node->parent->parent);
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
                    rightRotateHotel(T,node);
                }
                node->parent->color = 0;
                node->parent->parent->color = 1;
                leftRotateHotel(T,node->parent->parent);
            }
        }
    }
    T->root->color = 0;
    return;
}

void RBDeleteFixupHotel(RBHotel* T,Hotel* node){
    Hotel* w; //sibling of node
    while(node != T->root && node->color == 0){
        if(node == node->parent->left){ 
            w = node->parent->right;
            
            if(w->color == 1){ //case 1 ->case2,3,4
                w->color = 0;
                node->parent->color = 1;
                leftRotateHotel(T,node->parent);
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
                    rightRotateHotel(T,w);
                    w = node->parent->right;
                }
                w->color = node->parent->color;
                node->parent->color = 0;
                w->right->color = 0;
                leftRotateHotel(T,node->parent);
                node = T->root;
            }
        }
        else{ //node == node->parent->right
            w = node->parent->left;
            if(w->color == 1){ //case 1 ->case2,3,4
                w->color = 0;
                node->parent->color = 1;
                rightRotateHotel(T,node->parent);
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
                    leftRotateHotel(T,w);
                    w = node->parent->left;
                }
                w->color = node->parent->color; //case 4
                node->parent->color = 0;
                w->left->color = 0;
                rightRotateHotel(T,node->parent);
                node = T->root;
            }
        }
    }
    node->color = 0;
    return;
}

void RBinsertHotel(RBHotel* T, int cost, int id){ //insert by cost
    Hotel* parent = BSTsearchbycostHotel(T,cost);
    // if(parent->cost == cost) return; // 중복 값 허용
    Hotel* target = (Hotel*)malloc(sizeof(Hotel));
    target->left = T->NIL;
    target->right = T->NIL;
    target->cost = cost;
    target->id = id;
    target->color = 1; //initial color: red

    if(parent == T->NIL){
        T->root = target;
        target->parent = T->NIL;
    }

    if(parent->cost < cost)
        parent->right = target;
    else
        parent->left = target;    
    target->parent = parent;

    RBInsertFixupHotel(T,target);
    return;
}

Hotel* successorHotel(RBHotel* T, Hotel* target){ //return val otherwise return successor(val)
    Hotel* x;
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

void RBdeleteHotel(RBHotel* T,int cost){
    Hotel* target = BSTsearchbycostHotel(T,cost);
    if(target == T->NIL) return;
    Hotel* y; //node to splice out
    Hotel* x; //child of y
    
    if(target->left == T->NIL || target->right == T->NIL) //case 1,2: delete target
        y = target;
    else y = successorHotel(T,target); //case 3: delete successor

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
        target->cost = y->cost;
    
    if(y->color == 0)
        RBDeleteFixupHotel(T,x);
    free(y);
    return;
}

/*functions and structures for RBRESV*/

RBRESV* generateRBRESV(){
    RBRESV *T = (RBRESV*)malloc(sizeof(RBRESV));

    RESV* NILnode = (RESV*)malloc(sizeof(RESV));
    NILnode->parent = NULL;
    NILnode->left = NULL;
    NILnode->right = NULL;
    NILnode->color = 0; //NIL 노드 생성

    T->NIL = NILnode; 
    T->root = T->NIL; //노드 0개일 때 root는 NIL임
    return T;
}

void printBSTRESV(RBRESV* T,RESV* root,int space)
{
    if (root == T->NIL) return;

    space += 5;
    printBSTRESV(T,root->right, space);
    printf("\n");
    for (int i = 5; i < space; i++) printf(" ");
    if(root->color==0)
        printf("ID: %d(B)\n", root->id);
    else
        printf("ID: %d(R)\n",root->id);
    printBSTRESV(T,root->left, space);
}

void leftRotateRESV(RBRESV* T, RESV* node){
    RESV* y = node->right;
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

void rightRotateRESV(RBRESV* T, RESV* node){
    RESV* y = node->left;
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

RESV* BSTsearchRESV(RBRESV* T,int id){ 
    /*return exact node or it's parent(not exist)*/
    /*used for other RBtree insert/delete functions.*/
    RESV* x = T->root;
    if(x==T->NIL) return T->NIL;

    while(x->id != id){
        if(x->id > id  && x->left != T->NIL)
            x = x->left;
        else if(x->id < id && x->right != T->NIL)
            x = x->right;
        else break;
    }
    return x;
}

void RBInsertFixup(RBRESV* T,RESV* node){
    RESV* y; //uncle of node
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
                    leftRotateRESV(T,node);
                }
                node->parent->color = 0;
                node->parent->parent->color = 1;
                rightRotateRESV(T,node->parent->parent);
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
                    rightRotateRESV(T,node);
                }
                node->parent->color = 0;
                node->parent->parent->color = 1;
                leftRotateRESV(T,node->parent->parent);
            }
        }
    }
    T->root->color = 0;
    return;
}

void RBDeleteFixupRESV(RBRESV* T,RESV* node){
    RESV* w; //sibling of node
    while(node != T->root && node->color == 0){
        if(node == node->parent->left){ 
            w = node->parent->right;
            
            if(w->color == 1){ //case 1 ->case2,3,4
                w->color = 0;
                node->parent->color = 1;
                leftRotateRESV(T,node->parent);
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
                    rightRotateRESV(T,w);
                    w = node->parent->right;
                }
                w->color = node->parent->color;
                node->parent->color = 0;
                w->right->color = 0;
                leftRotateRESV(T,node->parent);
                node = T->root;
            }
        }
        else{ //node == node->parent->right
            w = node->parent->left;
            if(w->color == 1){ //case 1 ->case2,3,4
                w->color = 0;
                node->parent->color = 1;
                rightRotateRESV(T,node->parent);
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
                    leftRotateRESV(T,w);
                    w = node->parent->left;
                }
                w->color = node->parent->color; //case 4
                node->parent->color = 0;
                w->left->color = 0;
                rightRotateRESV(T,node->parent);
                node = T->root;
            }
        }
    }
    node->color = 0;
    return;
}

void RBinsertRESV(RBRESV* T, RESV* target){
    RESV* parent = BSTsearchRESV(T,target->id);
    // if(parent->id == id) return; // 중복 값 무시
    // RESV* target = (RESV*)malloc(sizeof(RESV));
    target->left = T->NIL;
    target->right = T->NIL;
    target->color = 1; //initial color: red

    if(parent == T->NIL){
        T->root = target;
        target->parent = T->NIL;
    }

    if(parent->id < target->id)
        parent->right = target;
    else
        parent->left = target;    
    target->parent = parent;

    RBInsertFixup(T,target);
    return;
}

RESV* successorRESV(RBRESV* T, RESV* target){ //return val otherwise return successor(val)
    RESV* x;
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

// RESV* BSTsearchprimeRESV(RBRESV* T, int id){ //return val otherwise return successor(val)
//     RESV* target = BSTsearchRESV(T,id);
//     if(target->id >= id) return target;
//     else return successorRESV(T,target);
    
// }

void RBdeleteRESV(RBRESV* T,int id){
    RESV* target = BSTsearchRESV(T,id);
    if(target == T->NIL) return;
    RESV* y; //node to splice out
    RESV* x; //child of y
    
    if(target->left == T->NIL || target->right == T->NIL) //case 1,2: delete target
        y = target;
    else y = successorRESV(T,target); //case 3: delete successor

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
        target->id = y->id;
    
    if(y->color == 0)
        RBDeleteFixupRESV(T,x);
    free(y);
    return;
}

void initialize_Hotel(SITE* site){
    site->Hotel = generateRBHotel();
    int cost;
    int minHotelcost_ = 130000;
    for(int i=0;i<100;i++){
        cost = rand()%100000+30000;
        if(cost < minHotelcost_) minHotelcost_ = cost;
        RBinsertHotel(site->Hotel,cost,i);
    }
    site->minHotelcost = minHotelcost_;
    return;
}

void initialize_SITE(){
    SITES = (SITE*)malloc(sizeof(SITE)*SITENUM);
    /*generate 100 sites*/
    for(int i=0;i<SITENUM;i++){
        SITES[i].id = i;
        SITES[i].duration = (rand()%5+1); // tour takes minimum 1 day to maximum 3 days.
        SITES[i].cost = 50000 + SITES[i].duration*(rand()%21+20)*1000; // 50000won for default, and add random weight proportion to duration.
        //additional cost is 20000won ~ 40000won /day.
        initialize_Hotel(SITES+i); //create Hotel RBtree.
    }
    /*set duration, cost for every sites*/
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
    int a,b;
    for(i=0;i<10;i++){
        for(j=0;j<10;j++){
            a = IndexMap[i][j];
            b = IndexMap[i][(j+1)%10];
            PATHS[a][b].connected = 1;
            PATHS[a][b].type = rand()%4;
            PATHS[a][b].cost = 2500*(PATHS[a][b].type)*(PATHS[a][b].type)+(PATHS[a][b].type+1)*rand()%4000+2000;
            PATHS[b][a].connected = 1;
            PATHS[b][a].type = PATHS[a][b].type;
            PATHS[b][a].cost = PATHS[a][b].cost;
        }
    }
    /*2. Between each (i)th and (i+1) row, make random 20 paths. */
    for(i=0;i<10;i++){
        for(j=0;j<20;j++){
            a = IndexMap[i%10][rand()%10];
            b = IndexMap[(i+1)%10][rand()%10];
            if(PATHS[a][b].connected == 1){
                j--;
                continue;
            }
            PATHS[a][b].connected = 1;
            PATHS[a][b].type = rand()%4;
            PATHS[a][b].cost = 2500*(PATHS[a][b].type)*(PATHS[a][b].type)+(PATHS[a][b].type+1)*rand()%4000+2000;
            PATHS[b][a].connected = 1;
            PATHS[b][a].type = PATHS[a][b].type;
            PATHS[b][a].cost = PATHS[a][b].cost;
        }
    }
    return;
}
void printpath(){
    for(int i=0;i<SITENUM;i++){
        for(int j=0;j<SITENUM;j++){
            printf("%d",PATHS[i][j].connected);
        }
        printf("\n");
    }
    return;
}

Hotel* find_hotel(SITE* site,int budget){ //always return hotel.
/*select hotel from possible list*/
        Hotel** hotellist = (Hotel**)calloc(100,sizeof(Hotel*));
        int i,validInput,intChoice;
        Hotel* userChoice;
        int hotelnum = -1; //index of last list element

        inorderFilterRBHotel(site->Hotel,site->Hotel->root,budget-site->cost,site->duration,&hotelnum,hotellist);
        printf("There are options for the hotels you could reserve.\n");
        printf("-------------------------------\n");
        for(i=0;i<=hotelnum;i++){
            printf("%02d. Hotel %02d:  %d won * %d night(s) = %d won.\n",i+1,hotellist[i]->id,hotellist[i]->cost,site->duration,hotellist[i]->cost*site->duration);
        }
        printf("\n-------------------------------\n");
        printf("Press choose hotel and enter hotel number: ");
        validInput = scanf("%d",&intChoice);
        
        while(1){
            if(validInput != 1){
                printf("Please enter numeric input. Try again: ");
                clearInputBuffer();
                validInput = scanf("%d",&intChoice);
                continue;
            }
            else if(intChoice<0){
                printf("Please enter one hotel within the list. Try again: ");
                clearInputBuffer();
                validInput = scanf("%d",&intChoice);
                continue;
            }
            for(i=0;i<=hotelnum;i++){
                if(hotellist[i]->id == intChoice){
                    userChoice = hotellist[i];
                    goto end;
                }
            }
            printf("Entered hotel doesn't exist. Try again: ");
            clearInputBuffer();
            validInput = scanf("%d",&intChoice);
            continue;
        }
        end:
        free(hotellist);
    
    return userChoice;
}

int find_next_dest(RESV* resv){
    //from site(depart), find candidates for next tour site considering leftover budget and dates.
    //Only prints out the list which user could choose for the next site.

    int i,sitenum,userChoice,validInput;
    /* make list which user can choose from.*/
    int* sitelist = (int*)calloc(SITENUM,sizeof(int)); 
    sitenum = -1; //index of last site in the list.
    
    if(resv->tour == NULL){ //if it is the first destination
        for(i=0;i<SITENUM;i++){
            if(resv->budget >= SITES[i].cost + SITES[i].minHotelcost*(SITES[i].duration-1) && resv->leftdays >= SITES[i].duration){
                sitenum++;
                sitelist[sitenum] = i; //id of site which is choosable
            }
        }
    }
    else{ //not the first destination
        for(i=0;i<SITENUM;i++){
            if(PATHS[resv->lastsite->id][i].connected == 1 && searchHistory(resv->tour,i) == 0){
                
                if(resv->budget >= PATHS[resv->lastsite->id][i].cost + SITES[i].cost + SITES[i].minHotelcost*SITES[i].duration
                && resv->leftdays >= SITES[i].duration){ //need enough budget and left days to reserve!
                    // the site has mininum 1 hotel which is reservable.
                    sitenum++;
                    sitelist[sitenum] = i;
                   
                }
            }
        }
    }

    
    if(sitenum == -1){
        printf("\n");
        printf("You have no choice for the next site. Continuing to the next process...\n");
        userChoice = -1;
    }
    else{
        printf("\n");
        printf("There are options for the sites you could proceed to in the following schedual.\n\n");
        printf("\n-------------------------------\n");
        for(i=0;i<=sitenum;i++){
            printf("%02d. Site %02d: %d day(s) schedual for %d won.\n",i+1,sitelist[i],SITES[sitelist[i]].duration,SITES[sitelist[i]].cost);
        }
        printf("\n-------------------------------\n");
        printf("Press site number or either -1 not to reserve further schedual: ");
        clearInputBuffer();
        validInput = scanf("%d",&userChoice);
        
        while(1){
            if(validInput != 1){
                printf("Please enter numeric input. Try again: ");
                clearInputBuffer();
                validInput = scanf("%d",&userChoice);
                continue;
            }
            if(userChoice == -1){
                free(sitelist);
                return -1;
            }

            for(i=0;i<=sitenum;i++){
                if(sitelist[i] == userChoice) goto end;
            }
            printf("Entered site doesn't exist. Try again: ");
            clearInputBuffer();
            validInput = scanf("%d",&userChoice);
            continue;
            }
    }
    end:
    free(sitelist);
    return userChoice;
}
 
void show_reservation(RESV* resv){
    TOUR* iter = resv->tour;
    int i=0;
    int daycount = resv->startdate;
    printf("User ID: %d\n",resv->id);
    printf("Tour Date: %s ~ %s\n",get_StrDate(resv->startdate),get_StrDate(resv->startdate+(resv->period-resv->leftdays)-1));
    printf("---- Tour schedual ----\n");

    while(iter!= NULL){
        printf("\n");
        printf("Destination %d: Site %02d --- %s ~ %s  cost: %d won\n",i+1,iter->cursite->id,get_StrDate(daycount),
                get_StrDate(daycount+iter->cursite->duration-1),iter->cursite->cost);
        daycount+=iter->cursite->duration;
        if(i!=0) printf("Tranportation: %s --- cost: %d won\n",get_trans(iter->path->type),iter->path->cost);
        printf("Lodging: Hotel %02d (%d day(s)) --- cost: %d won\n",iter->hotel->id,iter->cursite->duration,iter->hotel->cost*iter->cursite->duration);
        iter = iter->nextsite;
        i++;
    }
    printf("\n");
    printf("Total cost: %d won\n",resv->init_budget-resv->budget);
    printf("--------------------------\n");

    return;
}

int get_confirmation(RESV* resv,int option){
    char answer;
    int validInput;
    printf("\n\n");
    printf("** CONFIRMATION **\n\n");
    show_reservation(resv);
    if(option == 1) //reservation mode
        printf("Do you want to save this reservation? (Y/N): ");
    
    else if(option == 2) //cancel mode
        printf("Do you really want to cancel this reservation? (Y/N): ");

    clearInputBuffer();
    validInput = scanf("%c",&answer);
    while(validInput != 1 || !(answer == 'Y' || answer == 'N')){
        printf("Please enter again(Y/N): ");
        clearInputBuffer();
        validInput = scanf("%c",&answer);
    }
    return answer == 'Y' ? 1 : 0;
}

int seek_reservation(){
    /* return valid ID(reservation found) otherwise return -1. */
    int ID,validInput;

    printf("Enter reservation ID: ");
    do{ 
        clearInputBuffer();
        validInput = scanf("%d",&ID);
        if(validInput != 1 || BSTsearchRESV(RBresv,ID)->id != ID || ID > 9999){
            if(BSTsearchRESV(RBresv,ID)->id != ID){
                printf("Cannot Find reservation. Returning to main...\n");
                return -1;
            }
            else printf("Please enter proper ID. Try again:");
            continue;
        }
        else return ID;
    }while(1);
}

void start_reservation(){
    /*1.  search if the Id is not unique with search function*/
    int userid,validInput;
    
    printf("\nPlease enter reservation ID(up to 4-digits number): ");
    do{
        clearInputBuffer();
        validInput = scanf("%d",&userid);
        if(validInput != 1 || BSTsearchRESV(RBresv,userid)->id == userid || userid > 9999){ //If duplicated ID exist, search function return non-NIL
            if(BSTsearchRESV(RBresv,userid)->id == userid)
                printf("The id is already taken. Try other one: ");
            else
                printf("Please enter up to 4-digit integer. Try again: ");
        continue;
        }
    else break;
    }while(1);
    /* make resv node */
    RESV* resv = (RESV*)malloc(sizeof(RESV));
    resv->lastsite = NULL;
    resv->tour = NULL;

    /* 2. Get tour period, budget*/
    int month,day,period,budget;
    printf("\n");
    //starting date
    printf("From what month and date are you planing to depart? (ex) 7/25: ");
    do{
        clearInputBuffer();
        validInput = scanf("%d/%d",&month,&day);
        if(validInput != 2 || month<0 || month>12 || day<0 || day > daysformonth[month-1]){ 
            if(validInput != 2)
                printf("Please enter number for month and date. Try again (ex) 7/25: ");
            else
                printf("Entered date doesn't exist. Try again (ex) 7/25: ");
            continue;
        }

        else break;
    }while(1);
    printf("\n");
    //tour period
    printf("How many days do you plan to travel?: ");
    do{
        clearInputBuffer();
        validInput = scanf("%d",&period);
        if(validInput != 1 || period<1 || get_IntDate(month,day)+period >365){ 
            if(validInput != 1)
                printf("Please enter number for travel period. Try again: ");
            else if(period<1)
                printf("Enter positive number. Try again: ");
            else{
                printf("The reservation schedual is out of scope. Please choose within this year. Try again: ");
            }
        continue;
        }
        else break;
    }while(1);
    printf("\n");
    //budget
    printf("How much do you plan to spend on your tour? (won): ");
    do{
        clearInputBuffer();
        validInput = scanf("%d",&budget);
        if(validInput !=1 || budget < 0){ 
            printf("Please enter positive number for the input. Try again (ex) 400000: ");
            continue;
        }
        else break;
    }while(1);
    printf("\n");
    resv->id = userid;
    resv->startdate = get_IntDate(month,day);
    resv->init_budget = budget;
    resv->budget = budget;
    resv->period = period;
    resv->leftdays = period;

    /* 3. Select sites user want to visit. */

    int nextdest;
    Hotel* selectedhotel;
    do{
        nextdest = find_next_dest(resv);
        if(nextdest != -1){
            selectedhotel = find_hotel(&SITES[nextdest],resv->budget);
            insert_site(resv,nextdest,selectedhotel);
            resv->lastsite = &SITES[nextdest];
        }
    }while(nextdest != -1);
    if(resv->tour != NULL){
        if(get_confirmation(resv,1)==1){
            /*Add resv to the RBtree*/
            RBinsertRESV(RBresv,resv);
            printf("Reservation Complete! Proceeding to main... \n");
        }
        else{
            printf("Reservation Canceled. Proceeding to main... \n");
        }
    }
    else{
        printf("Nothing has been reserved. Proceeding to main... \n");
    }

    return;
}

void free_all(){
    /*free SITES*/
    free(SITES);
    /*free PATHS*/
    for(int i=0;i<SITENUM;i++){
        free(PATHS[i]);
    }
    free(PATHS);

    return;
}

int main(void){
    int option,validOption,ID;
    char buffer;
    srand((unsigned int)time(NULL));
    /*create RBtree for reservation*/
    RBresv = generateRBRESV();
    initialize_SITE(); //initialize sites and paths.
    initialize_PATH();
    printf("\n\n** Welcome to our tour reservation system! **");
   
   do{      
        printf("\n");
        printf("---------------------------\n");
        printf("1. Reserve Tour.\n");
        printf("2. View reservation list.\n");
        printf("3. View my reservation.\n");
        printf("4. Cancel reservation.\n");
        printf("5. Quit.\n");
        printf("---------------------------\n");
        printf("Select Option: ");
        validOption = scanf("%d",&option);

        if(validOption != 1 || option < 1 || option > 5){
            printf("\n\n");
            printf("Enter Proper option. Try again.\n");
            clearInputBuffer();
            continue;
        }

        switch(option){
            case 1:
                start_reservation();
                break;
            case 2:
                printBSTRESV(RBresv,RBresv->root,1);
                printf("\n\n");
                break;
            case 3:
                ID = seek_reservation();
                if(ID != -1)
                    show_reservation(BSTsearchRESV(RBresv,ID));
                break;
            case 4:
                ID = seek_reservation();
                if(ID != -1 && get_confirmation(BSTsearchRESV(RBresv,ID),2)==1){
                    RBdeleteRESV(RBresv,ID);
                    printf("Successfully Canceled.\n");
                }
                break;
            case 5:
                break;
        }
    }while(option != 5);
    printf("Thank you for using our Tour reservation system.\n");

    free_all();
    return 0;
}