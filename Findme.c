#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TABLE_SIZE 10
#define MAX 10
#define LOC 8

// STRUCT
typedef struct Item {
    int id;
    char desc[100];
    int location;
    struct Item* next;
} Item;

// GLOBALS

// LOST ITEMS (Linked List)
Item* lostHead = NULL;

// FOUND ITEMS (Hash Table)
Item* foundTable[TABLE_SIZE] = {NULL};

// QUEUE for claims
typedef struct {
    int data[MAX];
    int front, rear;
} Queue;

Queue q;

// LOCATIONS
char* locations[LOC] = {
    "Cultural Hall","Library","Food Court","IT Building",
    "Registrar Office","Gymnasium","OSAS Office","Gate"
};

// GRAPH for Dijkstra's algorithm
int graph[LOC][LOC] = {
    {0,2,4,0,0,0,0,7},
    {2,0,1,3,0,0,0,0},
    {4,1,0,2,5,0,0,0},
    {0,3,2,0,3,6,0,0},
    {0,0,5,3,0,2,4,0},
    {0,0,0,6,2,0,1,3},
    {0,0,0,0,4,1,0,2},
    {7,0,0,0,0,3,2,0}
};

// FUNCTION PROTOTYPES
int getInt();
void showLocations();
int hash(int key);
void addLost();
void viewLost();
void addFound();
void viewFound();
void searchItem();
void enqueue(int id);
void markClaimed();
void processClaim();
int minDist(int dist[], int vis[]);
void dijkstra(int src);
void cleanup();

// UTIL - Gets integer input from user
int getInt() {
    int x;
    while(scanf("%d",&x)!=1){
        printf("Invalid! Enter number: ");
        while(getchar()!='\n');
    }
    return x;
}

// Displays all campus locations
void showLocations() {
    int i;
    printf("\n--- LOCATIONS ---\n");
    for(i=0;i<LOC;i++)
        printf("%d. %s\n", i, locations[i]);
    printf("-----------------\n");
}

// HASH FUNCTION - Modulo division
int hash(int key){ 
    return key % TABLE_SIZE; 
}

// ADD LOST ITEM - Stores in linked list
void addLost() {
    Item* newItem = malloc(sizeof(Item));
    if(!newItem) {
        printf("Memory allocation failed!\n");
        return;
    }

    printf("\n--- ADD LOST ITEM ---\n");
    printf("Enter Lost Item ID: ");
    newItem->id = getInt();

    // Clear input buffer
    while(getchar() != '\n');
    
    printf("Description: ");
    fgets(newItem->desc, 100, stdin);
    
    // Remove trailing newline
    newItem->desc[strcspn(newItem->desc, "\n")] = 0;

    showLocations();
    printf("Enter location number (0-7): ");
    newItem->location = getInt();
    
    if(newItem->location < 0 || newItem->location >= LOC) {
        printf("Invalid location! Setting to Cultural Hall (0).\n");
        newItem->location = 0;
    }

    newItem->next = lostHead;
    lostHead = newItem;

    printf("? Lost item recorded successfully!\n");
}

// VIEW LOST ITEMS - Traverses linked list
void viewLost() {
    Item* temp;
    int count = 1;
    
    printf("\n--- LOST ITEMS ---\n");
    temp = lostHead;
    if(!temp){
        printf("No lost items in the system.\n");
        return;
    }
    
    while(temp){
        printf("\n[%d] ID: %d\n", count, temp->id);
        printf("    Description: %s\n", temp->desc);
        printf("    Lost at: %s\n", locations[temp->location]);
        printf("    ---\n");
        temp = temp->next;
        count++;
    }
}

// ADD FOUND ITEM - Stores in hash table
void addFound() {
    Item* newItem = malloc(sizeof(Item));
    int index;
    
    if(!newItem) {
        printf("Memory allocation failed!\n");
        return;
    }

    printf("\n--- ADD FOUND ITEM ---\n");
    printf("Enter Found Item ID: ");
    newItem->id = getInt();

    // Clear input buffer
    while(getchar() != '\n');
    
    printf("Description: ");
    fgets(newItem->desc, 100, stdin);
    
    // Remove trailing newline
    newItem->desc[strcspn(newItem->desc, "\n")] = 0;

    showLocations();
    printf("Enter location number (0-7): ");
    newItem->location = getInt();
    
    if(newItem->location < 0 || newItem->location >= LOC) {
        printf("Invalid location! Setting to Cultural Hall (0).\n");
        newItem->location = 0;
    }

    index = hash(newItem->id);
    newItem->next = foundTable[index];
    foundTable[index] = newItem;

    printf("? Found item added successfully!\n");
}

// VIEW FOUND ITEMS - Iterates through hash table
void viewFound() {
    int i;
    int empty = 1;
    Item* temp;
    
    printf("\n--- FOUND ITEMS ---\n");
    for(i=0;i<TABLE_SIZE;i++){
        temp = foundTable[i];
        while(temp){
            printf("\nID: %d\n", temp->id);
            printf("Description: %s\n", temp->desc);
            printf("Found at: %s\n", locations[temp->location]);
            printf("Hash Index: %d\n", i);
            printf("---\n");
            temp = temp->next;
            empty = 0;
        }
    }
    if(empty) printf("No found items in the system.\n");
}

// SEARCH ITEM - Searches both lost and found
void searchItem() {
    int id;
    int index;
    Item* f;
    Item* l;
    
    printf("\n--- SEARCH ITEM ---\n");
    printf("Enter ID to search: ");
    id = getInt();

    // Search in found items (hash table - O(1) average)
    index = hash(id);
    f = foundTable[index];
    while(f){
        if(f->id == id){
            printf("\n? ITEM FOUND (Status: FOUND)\n");
            printf("  ID: %d\n", f->id);
            printf("  Description: %s\n", f->desc);
            printf("  Location: %s\n", locations[f->location]);
            printf("  Status: Ready for claiming\n");
            return;
        }
        f = f->next;
    }

    // Search in lost items (linked list - O(n))
    l = lostHead;
    while(l){
        if(l->id == id){
            printf("\n? ITEM FOUND (Status: LOST)\n");
            printf("  ID: %d\n", l->id);
            printf("  Description: %s\n", l->desc);
            printf("  Location where lost: %s\n", locations[l->location]);
            printf("  Status: Not yet found\n");
            return;
        }
        l = l->next;
    }

    printf("\n? Item with ID %d not found in the system.\n", id);
}

// ENQUEUE - Adds claim to queue (FIFO)
void enqueue(int id){
    if(q.rear == MAX-1){ 
        printf("Claim queue is full! Cannot add more claims.\n"); 
        return;
    }
    if(q.front == -1) q.front = 0;
    q.rear++;
    q.data[q.rear] = id;
    printf("? Item ID %d added to claim queue (Position: %d).\n", id, q.rear+1);
}

// MARK CLAIMED - Adds found item to claim queue
void markClaimed(){
    int id;
    int index;
    Item* temp;
    int found = 0;
    
    printf("\n--- MARK ITEM FOR CLAIM ---\n");
    printf("Enter Found Item ID to claim: ");
    id = getInt();

    // Verify the item exists in found items before adding to queue
    index = hash(id);
    temp = foundTable[index];
    
    while(temp){
        if(temp->id == id){
            found = 1;
            break;
        }
        temp = temp->next;
    }
    
    if(!found){
        printf("? Item ID %d not found in found items. Cannot claim.\n", id);
        return;
    }

    enqueue(id);
}

// PROCESS CLAIM - Removes item from found items (FIFO)
void processClaim(){
    int id;
    int index;
    Item* temp;
    Item* prev;
    
    printf("\n--- PROCESS CLAIM ---\n");
    if(q.front == -1 || q.front > q.rear){
        printf("No claims to process at this time.\n"); 
        return;
    }

    id = q.data[q.front];
    q.front++;
    
    if(q.front > q.rear) {
        q.front = -1;
        q.rear = -1;
    }

    index = hash(id);
    temp = foundTable[index];
    prev = NULL;

    while(temp){
        if(temp->id == id){
            if(prev) 
                prev->next = temp->next;
            else 
                foundTable[index] = temp->next;

            printf("\n? CLAIM PROCESSED SUCCESSFULLY!\n");
            printf("  Item ID: %d\n", temp->id);
            printf("  Description: %s\n", temp->desc);
            printf("  Original found location: %s\n", locations[temp->location]);
            printf("  Status: Item has been returned to owner\n");
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }

    printf("? Error: Item ID %d not found in found items. Claim cannot be processed.\n", id);
}

// DIJKSTRA'S ALGORITHM - Find minimum distance vertex
int minDist(int dist[], int vis[]){
    int min = INT_MAX, idx = -1;
    int i;
    
    for(i = 0; i < LOC; i++)
        if(!vis[i] && dist[i] < min){
            min = dist[i]; 
            idx = i;
        }
    return idx;
}

// DIJKSTRA'S ALGORITHM - Find shortest paths from source
void dijkstra(int src){
    int dist[LOC], vis[LOC];
    int i, u, v;
    
    if(src < 0 || src >= LOC) {
        printf("Invalid source location!\n");
        return;
    }
    
    // Initialize distances
    for(i = 0; i < LOC; i++) {
        dist[i] = INT_MAX;
        vis[i] = 0;
    }
    dist[src] = 0;

    printf("\n--- CALCULATING SHORTEST PATHS ---\n");
    
    // Find shortest path for all vertices
    for(i = 0; i < LOC - 1; i++) {
        u = minDist(dist, vis);
        if(u == -1) break; // No reachable nodes left
        
        vis[u] = 1;

        for(v = 0; v < LOC; v++) {
            if(!vis[v] && graph[u][v] > 0 && 
               dist[u] != INT_MAX && 
               dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }

    // Print results
    printf("\n=== SHORTEST PATHS FROM %s ===\n", locations[src]);
    printf("%-20s -> %-20s %s\n", "FROM", "TO", "DISTANCE");
    printf("------------------------------------------------\n");
    
    for(i = 0; i < LOC; i++) {
        if(i == src) continue;
        if(dist[i] == INT_MAX)
            printf("%-20s -> %-20s %s\n", locations[src], locations[i], "UNREACHABLE");
        else
            printf("%-20s -> %-20s %d\n", locations[src], locations[i], dist[i]);
    }
    printf("================================================\n");
}

// CLEANUP - Frees all allocated memory
void cleanup() {
    int i;
    int lostCount = 0;
    int foundCount = 0;
    Item* current;
    Item* next;
    
    printf("\n--- CLEANING UP MEMORY ---\n");
    
    // Free lost items (linked list)
    current = lostHead;
    while(current) {
        next = current->next;
        free(current);
        current = next;
        lostCount++;
    }
    
    // Free found items (hash table chains)
    for(i = 0; i < TABLE_SIZE; i++) {
        current = foundTable[i];
        while(current) {
            next = current->next;
            free(current);
            current = next;
            foundCount++;
        }
    }
    
    printf("Cleaned up: %d lost items and %d found items.\n", lostCount, foundCount);
}

// MAIN FUNCTION
int main(){
    int choice;
    int src;
    
    // Initialize queue
    q.front = -1;
    q.rear = -1;
    
    printf("\n");
    printf("========================================\n");
    printf("   WELCOME TO FINDME\n");
    printf("   Campus Lost & Found System\n");
    printf("========================================\n");

    while(1){
        printf("\n+------------------------------------+\n");
        printf("¦         MAIN MENU                  ¦\n");
        printf("¦------------------------------------¦\n");
        printf("¦ 1.  Add Lost Item                  ¦\n");
        printf("¦ 2.  Add Found Item                 ¦\n");
        printf("¦ 3.  View Lost Items                ¦\n");
        printf("¦ 4.  View Found Items               ¦\n");
        printf("¦ 5.  Search Item                    ¦\n");
        printf("¦ 6.  Mark Item as Claimed           ¦\n");
        printf("¦ 7.  Process Claim Queue            ¦\n");
        printf("¦ 8.  Find Route (Dijkstra)          ¦\n");
        printf("¦ 9.  Exit System                    ¦\n");
        printf("+------------------------------------+\n");
        printf("Enter your choice: ");

        choice = getInt();

        switch(choice){
            case 1: 
                addLost(); 
                break;
            case 2: 
                addFound(); 
                break;
            case 3: 
                viewLost(); 
                break;
            case 4: 
                viewFound(); 
                break;
            case 5: 
                searchItem(); 
                break;
            case 6: 
                markClaimed(); 
                break;
            case 7: 
                processClaim(); 
                break;
            case 8: 
                showLocations();
                printf("Enter source location number (0-7): ");
                src = getInt();
                dijkstra(src);
                break;
            case 9: 
                printf("\nThank you for using the Lost & Found System!\n");
                cleanup();
                printf("Goodbye!\n");
                exit(0);
            default: 
                printf("\n? Invalid choice! Please enter a number between 1-9.\n");
        }
    }
    
    return 0;
}
