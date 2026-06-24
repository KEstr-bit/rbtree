#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rbtree.h"
#include "rbmap.h"


/* ---------- ключи/значения ---------- */

static char *new_str(const char *s)
{
    char *p = (char *)malloc(strlen(s) + 1);
    if (p) strcpy(p, s);
    return p;
}

static int *new_int(int v)
{
    int *p = (int *)malloc(sizeof(int));
    if (p) *p = v;
    return p;
}

static int cmp_str(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

static int cmp_int(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

static void free_str(void *p) { if (p) free(p); }
static void free_int(void *p) { if (p) free(p); }


/* ---------- печать формы дерева (сверху вниз, со связями) ---------- */

#define TW 8   /* ширина одной позиции по горизонтали */

typedef struct {
    RBNode *node;
    int pos;     /* порядковый номер при inorder-обходе (= колонка) */
    int level;
    int hl, hr;  /* есть левый/правый ребёнок */
    int lp, rp;  /* позиции детей */
} TN;

/* in-order сбор узлов с координатами; возвращает inorder-позицию узла */
static int tcollect(RBTree *t, RBNode *n, int lvl, TN *a, int *c, int *p)
{
    int idx, lpos, rpos;

    if (n == t->nil) {
        return -1;
    }
    idx = *c;                 /* слот узла резервируем до рекурсии */
    (*c)++;
    a[idx].node  = n;
    a[idx].level = lvl;
    a[idx].hl    = (n->left  != t->nil);
    a[idx].hr    = (n->right != t->nil);
    a[idx].lp    = -1;
    a[idx].rp    = -1;

    lpos = tcollect(t, n->left, lvl + 1, a, c, p);
    a[idx].pos = *p;
    (*p)++;
    rpos = tcollect(t, n->right, lvl + 1, a, c, p);

    a[idx].lp = lpos;         /* позиции детей берём из их возврата */
    a[idx].rp = rpos;
    return a[idx].pos;
}

typedef void (*kfmt)(RBNode *n, char *buf, size_t sz);

/* к ключу приписываем цвет узла: [К]=красный, [Ч]=чёрный */
static void fs(RBNode *n, char *buf, size_t sz)
{
    snprintf(buf, sz, "%s[%s]", (char *)n->key,
             n->color == RBN_RED ? "К" : "Ч");
}

static void fi(RBNode *n, char *buf, size_t sz)
{
    snprintf(buf, sz, "%d[%s]", *(int *)n->key,
             n->color == RBN_RED ? "К" : "Ч");
}

static void show_tree(RBTree *t, kfmt f)
{
    TN a[64];
    int c = 0, p = 0, maxl = 0;
    char line[1024];

    tcollect(t, t->root, 0, a, &c, &p);
    if (c == 0) {
        printf("  (пусто)\n");
        return;
    }
    for (int i = 0; i < c; i++) {
        if (a[i].level > maxl) maxl = a[i].level;
    }

    for (int lvl = 0; lvl <= maxl; lvl++) {
        /* ряд связей: '/' над левым ребёнком, '\' над правым */
        if (lvl > 0) {
            int last = 0;
            memset(line, ' ', sizeof(line));
            for (int i = 0; i < c; i++) {
                int x;
                if (a[i].level != lvl - 1) {
                    continue;
                }
                if (a[i].hl) {
                    x = a[i].lp * TW + TW / 2;
                    line[x] = '/';
                    if (x > last) last = x;
                }
                if (a[i].hr) {
                    x = a[i].rp * TW + TW / 2;
                    line[x] = '\\';
                    if (x > last) last = x;
                }
            }
            line[last + 1] = '\0';
            printf("%s\n", line);
        }
        /* ряд узлов (подписи центрируются по колонке) */
        {
            int last = 0;
            memset(line, ' ', sizeof(line));
            for (int i = 0; i < c; i++) {
                char b[24];
                int col, len;
                if (a[i].level != lvl) {
                    continue;
                }
                f(a[i].node, b, sizeof(b));
                len = (int)strlen(b);
                col = a[i].pos * TW + TW / 2 - len / 2;
                for (int k = 0; k < len; k++) {
                    line[col + k] = b[k];
                }
                if (col + len > last) {
                    last = col + len;
                }
            }
            line[last] = '\0';
            printf("%s\n", line);
        }
    }
}


/* ---------- map ---------- */

static void print_map(Map *m, const char *name)
{
    RBTreeIterator it = create_rbtree_iterator(m->tree);
    RBNode *n;
    void *k, *v;

    printf("%s = { ", name);
    while (rbtree_iterator_next(&it, &n, &k, &v)) {
        printf("%s:%s ", (char *)k, v ? (char *)v : "NULL");
    }
    printf("}\n");
}


/* ---------- тест: дерево ---------- */

static void test_tree(void)
{
    RBTree *t;
    void *r;

    printf("\nДЕРЕВО\n");

    /* --- строковые ключи --- */
    t = create_rbtree(cmp_str, free_str, free_str);

    rbtree_insert(t, new_str("c"), new_str("C"));
    rbtree_insert(t, new_str("a"), new_str("A"));
    rbtree_insert(t, new_str("b"), new_str("B"));
    rbtree_insert(t, new_str("e"), new_str("E"));
    rbtree_insert(t, new_str("d"), new_str("D"));
    printf("вставили c a b e d вразнобой, в дереве %u узлов.\n", get_rbtree_size(t));
    show_tree(t, fs);

    printf("мин=%s  макс=%s\n",
           (char *)rbtree_min(t)->key, (char *)rbtree_max(t)->key);
    printf("поиск b=%s  \n", (char *)rbtree_search(t, "b"));
    r = rbtree_search(t, "z");
    printf("поиск z=%s \n", r ? (char *)r : "нет");

    rbtree_insert(t, new_str("a"), new_str("A2"));
    printf("вставили a=A2 ещё раз — размер: %u,\n", get_rbtree_size(t));
    show_tree(t, fs);

    rbtree_remove(t, "b");
    rbtree_remove(t, "a");
    printf("удалили b и a, осталось %u узла:\n", get_rbtree_size(t));
    show_tree(t, fs);

    destroy_rbtree(t);

    /* --- пустое / один узел --- */
    t = create_rbtree(cmp_str, free_str, free_str);
    printf("\nпустое дерево: размер %u, мин=%p (искать нечего)\n",
           get_rbtree_size(t), (void *)rbtree_min(t));
    rbtree_insert(t, new_str("x"), new_str("X"));
    printf("один узел:\n");
    show_tree(t, fs);
    destroy_rbtree(t);

    /* --- целые ключи, масса вставок/удалений --- */
    t = create_rbtree(cmp_int, free_int, free_int);
    for (int i = 0; i < 20; i++) {
        rbtree_insert(t, new_int(i), new_int(i * 100));
    }
    for (int i = 0; i < 20; i += 2) {
        int k = i;
        rbtree_remove(t, &k);
    }
    printf("\nвставили 0..19 по порядку,\n");
    printf("Удалили все чётные. Осталось %u узлов.\n",
           get_rbtree_size(t));
    show_tree(t, fi);
    {
        RBTreeIterator it = create_rbtree_iterator(t);
        RBNode *n;
        void *k, *v;
        printf("обход inorder выдаёт ключи по возрастанию: ");
        while (rbtree_iterator_next(&it, &n, &k, &v)) {
            printf("%d ", *(int *)k);
        }
        printf("\n");
    }
    destroy_rbtree(t);

    /* --- join --- */
    {
        RBTree *t1 = create_rbtree(cmp_str, free_str, free_str);
        RBTree *t2 = create_rbtree(cmp_str, free_str, free_str);
        RBTree *j;

        rbtree_insert(t1, new_str("a"), new_str("A"));
        rbtree_insert(t1, new_str("c"), new_str("C"));
        rbtree_insert(t1, new_str("e"), new_str("E"));
        rbtree_insert(t2, new_str("g"), new_str("G"));
        rbtree_insert(t2, new_str("i"), new_str("I"));
        printf("\njoin: t1 = {a,c,e}, t2 = {g,i}, между ними кладём f\n");
        j = rbtree_join(t1, t2, new_str("f"), new_str("F"));
        printf("получили одно дерево из %u узлов (t1 и t2 при этом уничтожены):\n",
               get_rbtree_size(j));
        show_tree(j, fs);
        destroy_rbtree(j);
    }

    /* --- split --- */
    {
        RBTree *ts = create_rbtree(cmp_str, free_str, free_str);
        RBTree *L = NULL, *R = NULL;

        rbtree_insert(ts, new_str("1"), new_str("o"));
        rbtree_insert(ts, new_str("3"), new_str("t"));
        rbtree_insert(ts, new_str("5"), new_str("f"));
        rbtree_insert(ts, new_str("7"), new_str("s"));
        rbtree_insert(ts, new_str("9"), new_str("n"));
        printf("\nsplit: режем дерево {1,3,5,7,9} по ключу '5'\n");
        rbtree_split(ts, "5", &L, &R);
        printf("ключи <= '5' уходят влево, > '5' — вправо\n");
        printf("(в этой реализации сам '5' остаётся слева):\n");
        show_tree(L, fs);
        show_tree(R, fs);
        destroy_rbtree(L);
        destroy_rbtree(R);
    }
}


/* ---------- тест: map ---------- */

static void test_map(void)
{
    Map *m;
    char buf[32];

    printf("\nMAP\n");

    m = create_map(cmp_str, free_str, free_str);
    map_put(m, new_str("apple"),  new_str("red"));
    map_put(m, new_str("banana"), new_str("yellow"));
    map_put(m, new_str("cherry"), new_str("red"));
    map_put(m, new_str("apple"),  new_str("green"));   /* обновление */
    printf("вставили apple, banana, cherry и apple ещё раз.\n");
    printf("размер %u,\n",
           get_map_size(m));
    printf("а перезаписал значение red -> green:\n");
    print_map(m, "map");

    printf("apple=%s  banana? %s  lemon? %s\n",
           (char *)map_get(m, "apple"),
           does_map_contain(m, "banana") ? "да" : "нет",
           does_map_contain(m, "lemon")  ? "да" : "нет");
    printf("мин=%s  макс=%s\n",
           (char *)map_min(m)->key, (char *)map_max(m)->key);

    map_remove(m, "banana");
    printf("удалили banana, размер %u:\n", get_map_size(m));
    print_map(m, "map");
    destroy_map(m);

    /* --- целые ключи --- */
    m = create_map(cmp_int, free_int, free_int);
    int *k = new_int(42);
    map_put(m, k, new_int(100));
    printf("\nmap[int]: get(42)=%d\n", *(int *)map_get(m, k));
    map_remove(m, k);
    printf("после удаления ключа 42 размер %u\n", get_map_size(m));
    destroy_map(m);

    /* --- масштаб --- */
    m = create_map(cmp_str, free_str, free_str);
    for (int i = 0; i < 100; i++) {
        sprintf(buf, "key%03d", i);
        map_put(m, new_str(buf), new_str("v"));
    }
    for (int i = 0; i < 100; i += 2) {
        sprintf(buf, "key%03d", i);
        map_remove(m, buf);
    }
    printf("\nвставили 100 ключей key000..key099, удалили все чётные.\n");
    printf("осталось %u (половина). key050 на месте? %s, key051? %s\n",
           get_map_size(m),
           does_map_contain(m, "key050") ? "да" : "нет",
           does_map_contain(m, "key051") ? "да" : "нет");
    destroy_map(m);
}


int main(void)
{
    test_tree();
    test_map();

    return 0;
}