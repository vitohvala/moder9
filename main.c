#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/stat.h>

#define LIMIT 9
#define NEXT (LIMIT - 1)
#define DICTIONARY_FILE "dictionary_rs.txt"

typedef struct Node {
    char *word;
    struct Node *next[LIMIT];
} Node;

typedef struct xdata{
    Display *display;
    Window root;
    Window active_window;
    Atom property;
    long ev_mask;
} XData;


//Ova funkcija moze da bude bolja??? mozda
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
    } else if(prev == -48 ) {
        switch (s) {
            //     А          Б        Ц          Ч         Ћ
            case -112: case -111: case -90: case -89: case -117: return 0;
            //    а          б
            case -80:  case -79: return 0;
            //     Д          Е          Ф         Ђ
            case -108: case -107: case -92: case -126: return 1;
            //     д          е                
            case -76:  case -75: return 1;
            //     Г          Х         И
            case -109: case -91: case -104: return 2;
            //     г          и
            case  -77: case -72:            return 2;
            //     Ј          К          Л          Љ
            case -120: case -102: case -101: case -119: return 3; 
            //      к        л
            case  -70: case -69: return 3; 
            //     М         Н           Њ         О
            case -100: case -99:  case -118: case -98: return 4;
            //     м         н          о
            case -68:  case -67: case -66: return 4;
            //     П         Р          С          Ш          п                     
            case -97:  case -96:  case -95: case -88:  case -65:return 5;
            //     Т         У          В          в
            case -94:  case -93: case -110: case -78:  return 6;
            //      З          Ж         Џ         з          ж
            case -105: case -106: case -113: case -73: case -74: return 7;
        }
    } else if(prev == -47) {
        switch (s) {
            //     ц          ч           ћ
            case -122: case -121: case -101: return 0;
            //     ђ          ф
            case -110: case -124: return 1;
            //     х
            case -123: return 2;
            //     ј          љ
            case -104: case -103: return 3;
            //     њ
            case -102: return 4;
            //     р          с         ш
            case -128: case -127: case -120: return 5;
            //     т          у             
            case -126: case -125: return 6;
            // џ
            case -97: return 7;

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
    if((s > 31 && !isalnum(s))) return 8;
    return 9;
}

//funkcija koju treba doraditi
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

void print_recursive(Node *node) {

    if (node == NULL) return;
    if (node->word!= NULL) printf("%s\n", node->word);

    for (int i = 0; i < 9; i++) {
        print_recursive(node->next[i]);
    }
}

void starts_with(Node *root, char *prefix) {

    Node *current = root;

    for (size_t i = 0; i < strlen(prefix); i++) {
        int num = prefix[i] - '0' - 2;
        if (current->next[num] == NULL) return;
        current = current->next[num];
    }

    print_recursive(current);
}

int set_input(XData *xdata) {
    int format_return;
    XGetInputFocus(xdata->display, &xdata->active_window, &format_return); 
    if(xdata->active_window)
        XSelectInput(xdata->display, xdata->active_window, xdata->ev_mask);
    return format_return;
}

/* TODO:
 *      Error checking
 *      paste
 *      manji fajl koji ce da sadrzi reci koje se ponavljaju
 *      handle input
 * */



void xdata_init(XData *xdata){
    xdata->display = XOpenDisplay(NULL);
    xdata->root = DefaultRootWindow(xdata->display);
    xdata->property = XInternAtom(xdata->display, "_NET_ACTIVE_WINDOW", False);
    xdata->ev_mask = (KeyPressMask | KeyReleaseMask | FocusChangeMask);
    
    XSelectInput(xdata->display, xdata->root, xdata->ev_mask);
}


int main(int argc, char **argv) {
    
    FILE *dictionary = fopen(DICTIONARY_FILE, "r");
    if(!dictionary){
        perror("fopen");
        return 1;
    }
    clock_t start, end;
    double el;
    char str[100];
    Node *root = (Node *)calloc(1, sizeof(*root));
    Node *node = root;
    uint8_t skip_word = 0;
    start = clock();
    while(fgets(str, sizeof(str), dictionary)){
        int i;
        for(i = 0; i < strlen(str); i++){
            if(str[i] == '\n') break;
            if(str[i] == -60 || str[i] == -59 || str[i] == -48 || str[i] == -47) {
                get_index(str[i]);
                continue;
            }
            uint8_t index = get_index(str[i]);
            //printf("%d -- %d, %d, %s", index, str[i], i, str);
            if(index > 8) {
                skip_word = 1;
                break;
            }

            if(node->next[index] == NULL){
                Node *new = (Node *)calloc(1, sizeof(*new));
                node->next[index] = new;
            }
           node = node->next[index];
        }
        if(skip_word) {
            skip_word = 0;
            continue;
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
    end = clock();
    el = (double) (end - start) / CLOCKS_PER_SEC;

file_err:
    fclose(dictionary);
    printf("Finished %lf\n", el);
    
    char buffer[100];
    size_t ind = 0;

    XData xdata = {0};
    xdata_init(&xdata);

    while(1){
        set_input(&xdata);
        XEvent ev = {0};
        XNextEvent(xdata.display, &ev);
        if(ev.type == KeyPress){
            KeySym keysym = XLookupKeysym(&ev.xkey, 0);
            if(keysym == XK_q) break; 
            else if((keysym > XK_KP_1 && keysym < XK_KP_9) ||
                    (keysym > XK_1 && keysym < XK_9))
                buffer[ind++] = (char) keysym;
        }
        if(ev.type == FocusOut){
            set_input(&xdata);
        } 
        //nanosleep(&ts, NULL);
    }
    Atom clipboard = XInternAtom(xdata.display, "CLIPBOARD", False);
    Atom targetType = XInternAtom(xdata.display, "UTF8_STRING", False);
    XConvertSelection(xdata.display, clipboard, targetType, None, xdata.active_window, CurrentTime);
    XFlush(xdata.display);
    start = clock();
    buffer[ind] = '\0';
    puts(buffer);
    printf("\n%s\n", get_word(&root, buffer));
    strcpy(str, get_word(&root, "0"));
    while (str[0] != '1') {
        printf("%s\n", str);
        strcpy(str, get_word(&root, "0"));
    }
    end = clock();
    el = (double) (end - start) / CLOCKS_PER_SEC;
    printf("Finished searching %lf\n", el);
    XCloseDisplay(xdata.display);
err:
    node_free(root);
    return 0;
}
