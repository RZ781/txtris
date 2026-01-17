#ifndef BACKEND_H
#define BACKEND_H

#define K_LEFT  (-1)
#define K_RIGHT (-2)
#define K_UP    (-3)
#define K_DOWN  (-4)

typedef struct {
       int x;
       int y;
       int width;
       int height;
       void* backend_data;
} Window;

typedef struct Backend {
       void (*init)(void);
       void (*exit)(void);
       int (*get_key)(int);
       void (*init_window)(Window*);
       void (*full_update)(void);
       void (*update)(Window);
       void (*print)(int, int, const char*, ...);
       void (*erase_window)(Window);
       void (*erase_line)(int, int);
       void (*draw_cell)(Window, int, int, int);
       void (*draw_box)(Window);
} Backend;

#endif
