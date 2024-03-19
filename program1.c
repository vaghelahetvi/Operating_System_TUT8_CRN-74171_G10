#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure definition for process
typedef struct {
    char parent[256]; // Name of the parent process
    char name[256]; // Name of the process
    int priority; // Priority of the process
    int memory; // Memory used by the process
} proc;

// Structure definition for binary tree node
typedef struct node {
    proc data;
    struct node* left;
    struct node* right;
} Node;

// Function to create a new node
Node* createNode(proc data) {
    Node* newNode = (Node*)malloc(sizeof(Node)); // Allocate memory for the new node
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data; // Set the data of the new node
    newNode->left = newNode->right = NULL; // Initialize left and right pointers to NULL
    return newNode;
}

// Function to insert a node into the binary tree
void insert(Node** root, proc data) {
    if (*root == NULL) {
        *root = createNode(data); // Create a new node if the tree is empty
    } else {
        // Insert the node recursively based on the process name
        if (strcmp(data.parent, "") == 0) {
            // If parent name is empty, set it to "NULL"
            strcpy(data.parent, "NULL");
        }
        if (strcmp(data.parent, (*root)->data.name) <= 0) {
            insert(&((*root)->left), data);
        } else {
            insert(&((*root)->right), data);
        }
    }
}

// Function to print the contents of the binary tree using inorder traversal
void printTree(Node* root) {
    if (root != NULL) {
        printTree(root->left); // Traverse the left subtree
        // Print the details of the current node
        printf("Parent: %s, Name: %s, Priority: %d, Memory: %d\n", root->data.parent, root->data.name, root->data.priority, root->data.memory);
        printTree(root->right); // Traverse the right subtree
    }
}

int main(void) {
    FILE* file = fopen("processes_tree.txt", "r"); // Open the file for reading
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n"); // Print an error message if the file cannot be opened
        exit(EXIT_FAILURE);
    }

    Node* root = NULL; // Initialize the root of the binary tree
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        proc newProc;
        // Read the data from each line of the file and insert it into the binary tree
        sscanf(line, "%[^,], %[^,], %d, %d", newProc.parent, newProc.name, &newProc.priority, &newProc.memory);
        insert(&root, newProc);
    }
    fclose(file); // Close the file after reading

    printf("Binary Tree Contents:\n");
    printTree(root); // Print the contents of the binary tree using inorder traversal

    // Free allocated memory
    // You need to implement a function to free the tree nodes

    return 0;
}

