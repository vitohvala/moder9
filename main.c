#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/stat.h>

#define LIMIT 9
#define NEXT (LIMIT - 1)

typedef struct Node {
    char *word;
    struct Node *next[LIMIT];
} Node;

struct input_event ev;
int fd;
uint8_t get_key(int fd) {
    read(fd, &ev, sizeof(ev));
    if (ev.type == EV_KEY && ev.value == 0) {
        switch (ev.code) {
            case 2: return  '1';
            case 3: return  '2';
            case 4: return  '3';
            case 5: return  '4';
            case 6: return  '5';
            case 7: return  '6';
            case 8: return  '7';
            case 9: return  '8';
            case 10: return '9';
            case 11: return '0';
        }
    }
    return 10;

} 

uint8_t get_index(char s) {
    
    static char prev;
    if(prev == -60) {
        switch (s) {
            case -115: case -116: case -121: case -122: return 0;
            case -111: case -112: return 1;
        }
    } else if(prev == -59){
        switch (s) {
            case -66: case -67: return 7;
            case -95: case -96: return 5;
        }
    }
            
    prev = s;
    switch(s) {
        case 'a': case 'b': case 'c':           return 0;
        case 'A': case 'B': case 'C':           return 0;
        case 'd': case 'e': case 'f':           return 1;
        case 'D': case 'E': case 'F':           return 1;
        case 'g': case 'h': case 'i':           return 2;
        case 'G': case 'H': case 'I':           return 2;
        case 'j': case 'k': case 'l':           return 3;
        case 'J': case 'K': case 'L':           return 3;
        case 'm': case 'n': case 'o':           return 4;
        case 'M': case 'N': case 'O':           return 4;
        case 'p': case 'q': case 'r': case 's': return 5;
        case 'P': case 'Q': case 'R': case 'S': return 5;
        case 't': case 'u': case 'v':           return 6;
        case 'T': case 'U': case 'V':           return 6;
        case 'w': case 'x': case 'y': case 'z': return 7;
        case 'W': case 'X': case 'Y': case 'Z': return 7;
    }
    if(s > 31 && !isalnum(s)) return 8;
    return 9;
}

char *get_word(Node **current, char *numbers) {
    if(numbers[0] == '0') {
        if((*current)->next[NEXT] == NULL) return "1";
        (*current) = (*current)->next[NEXT];
        return (*current)->word;
    }
    for(size_t i = 0; i< strlen(numbers); i++){
        int num = numbers[i] - '0' - 2;
        if((*current)->next[num] == NULL) return "Ne postoji";
        else (*current) = (*current)->next[num];
    } 
    if((*current) == NULL || (*current)->word == NULL) return "Ne postoji";
    else  return (*current)->word;
}

void node_free(Node* root) {
  for (int i = 0; i < 9; i++) {
    if (root->next[i] != NULL) {
      node_free(root->next[i]);
    }
  }
  free(root->word);
  free(root);
}

int main(int argc, char **argv) {
    
    FILE *dictionary = fopen("dict.txt", "r");
    if(!dictionary){
        perror("fopen");
        return 1;
    }
    char str[100];
    Node *root = (Node *)calloc(1, sizeof(*root));
    Node *node = root;
    while(fgets(str, sizeof(str), dictionary)){
        int i;
        for(i = 0; i < strlen(str); i++){
            if(str[i] == '\n') break;
            if(str[i] == -60 || str[i] == -59){
                get_index(str[i]);
                continue;
            }
            uint8_t index = get_index(str[i]);
            if(index == 9) {
                printf("%d\n", str[i]);
                goto file_err;
            }

            if(node->next[index] == NULL){
                Node *new = (Node *)calloc(1, sizeof(*new));
                node->next[index] = new;
            }
            node = node->next[index];
        }
        int str_len = strlen(str);
        str[str_len - 1] = '\0';
        char *word = (char *) malloc(str_len * sizeof(char));
        if(word == NULL){
            perror("malloc");
            break;
        }
        strncpy(word, str, str_len);
        if(node->word == NULL){
            node->word = word;
        } else {
            while (node->next[NEXT] != NULL) {
                node = node->next[NEXT];
            }
            Node* new = (Node*)calloc(1, sizeof(Node));
            new->word = word;
            node->next[NEXT] = new;
        }
        node = root;
    } 

file_err:
    fclose(dictionary);
    printf("Zavrseno\n");
    
    fd = open("/dev/input/event3", O_RDONLY);
    if(fd < 0) goto err;
    char buffer[100];
    size_t ind = 0;
    while(1){
        uint8_t key = get_key(fd);
        if(key > '1' && key <= '9') buffer[ind++] = key;
        if(key == '0') break;
    }

    buffer[ind] = '\0';
    printf("%s\n", buffer);
    printf("%s\n", get_word(&root, buffer));
    char tmp[100];
    while(tmp[0] != '1'){
        strcpy(tmp, get_word(&root, "0"));
        if(tmp[0] != '1') printf("%s\n", tmp);
    }
err:
    node_free(node);
    return 0;
}
