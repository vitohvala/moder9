#include <X11/X.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <limits.h>

#define LIMIT 9
#define NEXT (LIMIT - 1)

#define DEFER(p, s, which_err) \
    if(p == NULL) { \
        perror(s); \
        if(dictionary != NULL) fclose(dictionary); \
        goto which_err; }   

#define FREE_2DARRAY(arr, len) \
    do { \
        for(size_t i = 0; i < len; i++){ \
            free(arr[i]); \
        } \
        free(arr);\
    } while(0)


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
            //    џ
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
    if(numbers[0] == '0'){
        if((*current)->next[NEXT] == NULL) return NULL;
        (*current) = (*current)->next[NEXT];
        return (*current)->word;
    }
    for(size_t i = 0; i< strlen(numbers); i++){
        int num = numbers[i] - '0' - 2;
        if((*current)->next[num] == NULL) {
            return NULL;
        }
        else (*current) = (*current)->next[num];
    }
    return (*current)->word;
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

void set_input(XData *xdata) {
    int format_return;
    Window wintmp = xdata->active_window;
    XGetInputFocus(xdata->display, &wintmp, &format_return);
	
    if(wintmp != 1 && format_return != RevertToNone){
	    xdata->active_window = wintmp;
	    XSelectInput(xdata->display, xdata->active_window, xdata->ev_mask);
    }
}

/* TODO:
 *      Error checking
 *      pasteq
 *      manji fajl koji ce da sadrzi reci koje se ponavljaju
 *      handle input
 * */

uint8_t xdata_init(XData *xdata){
    xdata->display = XOpenDisplay(NULL);
    if(xdata->display == NULL) return 1;
    xdata->root = DefaultRootWindow(xdata->display);
    xdata->property = XInternAtom(xdata->display, "_NET_ACTIVE_WINDOW", False);
    if(xdata->property == BadAlloc || xdata->property == BadValue) return 1;
    xdata->ev_mask = (KeyPressMask | KeyReleaseMask | FocusChangeMask);

    xdata->active_window = XCreateSimpleWindow(xdata->display, xdata->root, 0, 0, 1, 1, 0, 0, 0);

    if(XSelectInput(xdata->display, xdata->root, xdata->ev_mask) == BadWindow) return 1;

    return 0;
}

void usage(void) {
    puts("Usage:");
    puts("[Command] [DICTIONARY_FILE] [OPTION]");

}
void send_key_event(XData *xdata, KeySym keysym, Bool m, long Mask) {
    XKeyEvent event;
    event.display = xdata->display;
    event.window = xdata->active_window;
    event.root = DefaultRootWindow(xdata->display);
    event.subwindow = None;
    event.time = CurrentTime;
    event.x = 0;
    event.y = 0;
    event.x_root = 0;
    event.y_root = 0;
    event.same_screen = 1;
    event.type = m ? KeyPress : KeyRelease;
    event.keycode = XKeysymToKeycode(xdata->display, keysym);
    event.state = Mask;

    XSendEvent(xdata->display, xdata->active_window, 1, KeyPressMask, (XEvent *)&event);
    XFlush(xdata->display);
}
int main(int argc, char **argv) {

    if(argc < 2) {
        usage();
        return 0;
    }

    FILE *dictionary = fopen(argv[1], "r");
    if(!dictionary){
        perror("fopen");
        return 1;
    }
    clock_t start, end;
    double el;
    char str[100];
    Node *root = (Node *)calloc(1, sizeof(*root));
    if(root == NULL) {
        perror("calloc");
        fclose(dictionary);
        return 1;
    }
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
                DEFER(new, "calloc", node_err);
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
        DEFER(word, "malloc", node_err); 

        strncpy(word, str, str_len);
        DEFER(word, "strncpy", node_err);

        if(node->word == NULL){
            node->word = word;
        } else {
            while (node->next[NEXT] != NULL) {
                node = node->next[NEXT];
            }
            Node* new = (Node*)calloc(1, sizeof(Node));
            DEFER(new, "calloc", node_err);
            new->word = word;
            node->next[NEXT] = new;
        }
        node = root;
    } 

    end = clock();
    el = (double) (end - start) / CLOCKS_PER_SEC;

    fclose(dictionary);
    printf("Finished %lf\n", el);

    XData xdata = {0};
    if(xdata_init(&xdata)){
        perror("X11");
        goto x_err;
    }
    char buffer[100];
    size_t ind = 0;
    size_t prev_ind = 0;
    Atom clipboard = XInternAtom(xdata.display, "CLIPBOARD", False);
    Atom targets = XInternAtom(xdata.display, "TARGETS", False);
    Atom UTF8 = XInternAtom(xdata.display, "UTF8_STRING", False);
    Atom text = XInternAtom(xdata.display, "TEXT", False);

    size_t suggested_size = 0;
    char **suggested = (char **) malloc(sizeof(char *) * 10);
    DEFER(suggested, "malloc", sug_err);
    size_t capacity = 10;
    int selected = -1;
    XSetSelectionOwner(xdata.display, clipboard, xdata.root, CurrentTime);
    if(XGetSelectionOwner(xdata.display, clipboard) != xdata.root) printf("ne radi");
    Atom xclip = XInternAtom(xdata.display, "XSEL_DATA", False);
    Atom target;
    Atom TARGETS = XInternAtom(xdata.display , "TARGETS", False);
    //XConvertSelection(xdata.display, clipboard, TARGETS, xclip, xdata.active_window, CurrentTime);
    XEvent ev = {0};
    set_input(&xdata);
    Window st_win = xdata.active_window;
    while(1){
	set_input(&xdata);
        XNextEvent(xdata.display, &ev);
        
        if(ev.type == KeyPress){
            KeySym keysym = XLookupKeysym(&ev.xkey, 0);
            if(keysym == XK_q) break;
            else if(keysym == XK_1) {
                buffer[ind] = '\0';
                if(selected < 0){
                    root = node;
                    prev_ind = ind;
                    printf("%s\n", buffer);
                    selected = 0;
                    while(1){
                        char *word = get_word(&root, buffer);
                        if(word == NULL) {
                            if(suggested_size < 1) selected = -1;
                            break;
                        }
                        int word_size = strlen(word);
                        word[word_size] = '\0';
                        suggested[suggested_size] = (char *) malloc(sizeof(char) * word_size + 1);
                        DEFER(suggested[suggested_size], "malloc", sug_err); 

                        strncpy(suggested[suggested_size], word, word_size + 1);
                        DEFER(suggested[suggested_size], "strncpy", sug_err);
                        suggested_size += 1;
                        if(suggested_size >= capacity){
                            capacity += 10;
                            char **ptr_to_s = (char **)realloc(suggested, sizeof(char *) * capacity);
                            DEFER(ptr_to_s, "realloc", sug_err);
                            suggested = ptr_to_s;
                        }
                        buffer[0] = '0';
                        buffer[1] = '\n';
                        ind = 0;
                    }
                }
                if(selected > -1){
                    printf("\nsuggested size: %zu\nArray:\n", suggested_size);
                    for (int i = 0; i < suggested_size; i++) {
                        printf("[%s] ", suggested[i]);
                    }

                    printf("prev_ind = %zu\n", prev_ind);
                    for(int i = 0; i < prev_ind + 1; i++){
                        send_key_event(&xdata, XK_BackSpace, 1, 0);
                        send_key_event(&xdata, XK_BackSpace, 0, 0);
                    }
                    puts("");
                    printf("%s\n", suggested[selected++]);
                    if(selected > suggested_size - 1) selected = 0;
                    send_key_event(&xdata, XK_v, 1, ControlMask | ShiftMask);
                    send_key_event(&xdata, XK_v, 0, ControlMask | ShiftMask);
                    //XConvertSelection(xdata.display, clipboard, targets, xclip,
                    //                    xdata.active_window, CurrentTime);
                    //XSendEvent(xdata.display, xdata.root, False, 0, &event);
                    XFlush(xdata.display);
                }
            } else if ((keysym > XK_KP_1 && keysym <= XK_KP_9) || (keysym > XK_1 && keysym <= XK_9))
            {  
                if (suggested_size > 0) {
                    FREE_2DARRAY(suggested, suggested_size);
                    suggested_size = 0;
                    selected = -1;
                    prev_ind = 0;
                    capacity = 10;
                    suggested = (char **)malloc(sizeof(char *) * capacity);
                }
                if (ind >= 99) ind = 0;
                selected = -1; 
                buffer[ind++] = (char) keysym;
            }
            if(keysym == XK_BackSpace){
                if(ind > 1) ind--;
            }
        } else if(ev.type == FocusOut){
            set_input(&xdata);
	    //puts("FocusOut");
        } else if(ev.type == FocusIn) {
	    //puts("FocusIn");
            set_input(&xdata);
	}
        if(ev.type == SelectionRequest){
            puts("SelectionRequest");
	    if (ev.xselectionrequest.selection == clipboard && selected > -1){
	    XSelectionRequestEvent  request = ev.xselectionrequest;
		    XChangeProperty(request.display, request.requestor, request.property, request.target, 8, 
				    PropModeReplace, (unsigned char *)suggested[selected], strlen(suggested[selected]));
		    XSelectionEvent sendEvent;
		    sendEvent.type = SelectionNotify;
		    sendEvent.serial = request.serial;
		    sendEvent.send_event = request.send_event;
		    sendEvent.display = request.display;
		    sendEvent.requestor = request.requestor;
		    sendEvent.selection = request.selection;
		    sendEvent.target = request.target;
		    sendEvent.property = request.property;
		    sendEvent.time = request.time;
             

		    if(XSendEvent (xdata.display, sendEvent.requestor, 0, 0, (XEvent *)&sendEvent) == 0)
			    puts("Pao");
		    //XFlush(xdata.display);q
	    }
        }

        if(ev.type == SelectionNotify){
            puts("SelectionNotify");
        }
	if(ev.type == DestroyNotify){
	    puts("Destroy");
            set_input(&xdata);
	}
    }

sug_err:
    FREE_2DARRAY(suggested, suggested_size);
x_err:
    XCloseDisplay(xdata.display);
node_err:
    node_free(node);
    return 0;
}
