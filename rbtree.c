#include <stdlib.h>
#include <stdio.h>
#include "rbtree.h"

/* Создание узла */
static RBNode *create_node(RBTree *tree, RBNodeColor color, void *key, void *value)
{
    RBNode *node;

    node = (RBNode *)malloc(sizeof(RBNode));
    if (node == NULL) {
        return NULL;
    }

    node->key   = key;
    node->value = value;
    node->color = color;
    node->parent = tree->nil;
    node->left  = tree->nil;
    node->right = tree->nil;

    return node;
}

/* Левый поворот */
static void left_rotate(RBTree *tree, RBNode **root, RBNode *x)
{
    RBNode *y;

    y = x->right;
    x->right = y->left;

    if (y->left != tree->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == tree->nil) {
        *root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

/* Правый поворот */
static void right_rotate(RBTree *tree, RBNode **root, RBNode *x)
{
    RBNode *y;

    y = x->left;
    x->left = y->right;

    if (y->right != tree->nil) {
        y->right->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == tree->nil) {
        *root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }

    y->right = x;
    x->parent = y;
}

/* Восстановление свойств RB-дерева после вставки */
static void insert_fixup(RBTree *tree, RBNode **root, RBNode *t)
{
    RBNode *parent;
    RBNode *grandfather;
    RBNode *uncle;

    if (t == *root) {
        t->color = RBN_BLACK;
        return;
    }

    while (t->parent->color == RBN_RED) {
        parent      = t->parent;
        grandfather = parent->parent;

        if (parent == grandfather->left) {
            /* Отец — левый ребёнок дедушки */
            uncle = grandfather->right;

            if (uncle->color == RBN_RED) {
                /* Случай 1: дядя красный */
                parent->color      = RBN_BLACK;
                uncle->color       = RBN_BLACK;
                grandfather->color = RBN_RED;
                t = grandfather;
            } else {
                /* Случай 2 и 3: дядя чёрный */
                if (t == parent->right) {
                    /* Случай 2: внутренний внук */
                    t = parent;
                    left_rotate(tree, root, t);
                    parent      = t->parent;
                    grandfather = parent->parent;
                }
                /* Случай 3: внешний внук */
                parent->color      = RBN_BLACK;
                grandfather->color = RBN_RED;
                right_rotate(tree, root, grandfather);
            }
        } else {
            /* Отец — правый ребёнок дедушки */
            uncle = grandfather->left;

            if (uncle->color == RBN_RED) {
                parent->color      = RBN_BLACK;
                uncle->color       = RBN_BLACK;
                grandfather->color = RBN_RED;
                t = grandfather;
            } else {
                if (t == parent->left) {
                    t = parent;
                    right_rotate(tree, root, t);
                    parent      = t->parent;
                    grandfather = parent->parent;
                }
                parent->color      = RBN_BLACK;
                grandfather->color = RBN_RED;
                left_rotate(tree, root, grandfather);
            }
        }
    }

    (*root)->color = RBN_BLACK;
}

/* Восстановление свойств после удаления */
static void delete_fixup(RBTree *tree, RBNode *p)
{
    RBNode *parent;
    RBNode *brother;

    while (p->color == RBN_BLACK && p != tree->root) {
        parent = p->parent;

        if (p == parent->left) {
            brother = parent->right;

            if (brother->color == RBN_RED) {
                /* Брат красный */
                brother->color = RBN_BLACK;
                parent->color  = RBN_RED;
                left_rotate(tree, &tree->root, parent);
                brother = parent->right;
            }

            if (brother->left->color  == RBN_BLACK &&
                brother->right->color == RBN_BLACK) {
                brother->color = RBN_RED;
                p = parent;
            } else {
                if (brother->right->color == RBN_BLACK) {
                    brother->left->color = RBN_BLACK;
                    brother->color       = RBN_RED;
                    right_rotate(tree, &tree->root, brother);
                    brother = parent->right;
                }
                brother->color        = parent->color;
                parent->color         = RBN_BLACK;
                brother->right->color = RBN_BLACK;
                left_rotate(tree, &tree->root, parent);
                p = tree->root;
            }
        } else {
            brother = parent->left;

            if (brother->color == RBN_RED) {
                brother->color = RBN_BLACK;
                parent->color  = RBN_RED;
                right_rotate(tree, &tree->root, parent);
                brother = parent->left;
            }

            if (brother->right->color == RBN_BLACK &&
                brother->left->color  == RBN_BLACK) {
                brother->color = RBN_RED;
                p = parent;
            } else {
                if (brother->left->color == RBN_BLACK) {
                    brother->right->color = RBN_BLACK;
                    brother->color        = RBN_RED;
                    left_rotate(tree, &tree->root, brother);
                    brother = parent->left;
                }
                brother->color       = parent->color;
                parent->color        = RBN_BLACK;
                brother->left->color = RBN_BLACK;
                right_rotate(tree, &tree->root, parent);
                p = tree->root;
            }
        }
    }

    p->color = RBN_BLACK;
    if (tree->root != tree->nil) {
        tree->root->color = RBN_BLACK;
    }
}

/* Вычисление чёрной высоты поддерева */
static int bh(RBTree *tree, RBNode *x)
{
    int h = 0;

    while (x != tree->nil) {
        x = x->left;
        if (x == tree->nil || x->color == RBN_BLACK) {
            h++;
        }
    }
    return h;
}

/* Вспомогательные функции для join (поиск узла на нужной чёрной высоте) */
static RBNode *find_right(RBTree *tree, RBNode *T, int h, RBNode **parent_out)
{
    int curBH = bh(tree, T);
    RBNode *curV = T;
    RBNode *P = tree->nil;

    while (curBH != h) {
        P = curV;
        curV = curV->right;
        if (curV->color == RBN_BLACK) {
            --curBH;
        }
    }
    *parent_out = P;
    return curV;
}

static RBNode *find_left(RBTree *tree, RBNode *T, int h, RBNode **parent_out)
{
    int curBH = bh(tree, T);
    RBNode *curV = T;
    RBNode *P = tree->nil;

    while (curBH != h) {
        P = curV;
        curV = curV->left;
        if (curV->color == RBN_BLACK) {
            --curBH;
        }
    }
    *parent_out = P;
    return curV;
}

static RBNode *join_to_right(RBTree *tree, RBNode *T1, RBNode *T2, RBNode *k)
{
    RBNode *P;
    RBNode *Y;
    RBNode *root = T1;

    Y = find_right(tree, T1, bh(tree, T2), &P);

    k->color  = RBN_RED;
    k->left   = Y;
    k->right  = T2;
    k->parent = P;

    if (Y != tree->nil) {
        Y->parent = k;
    }
    if (T2 != tree->nil) {
        T2->parent = k;
    }

    if (P == tree->nil) {
        root = k;
    } else {
        P->right = k;
    }

    insert_fixup(tree, &root, k);
    return root;
}

static RBNode *join_to_left(RBTree *tree, RBNode *T1, RBNode *T2, RBNode *k)
{
    RBNode *P;
    RBNode *Y;
    RBNode *root = T2;

    Y = find_left(tree, T2, bh(tree, T1), &P);

    k->color  = RBN_RED;
    k->left   = T1;
    k->right  = Y;
    k->parent = P;

    if (T1 != tree->nil) {
        T1->parent = k;
    }
    if (Y != tree->nil) {
        Y->parent = k;
    }

    if (P == tree->nil) {
        root = k;
    } else {
        P->left = k;
    }

    insert_fixup(tree, &root, k);
    return root;
}

/* Объединение двух деревьев с разделительным узлом k */
static RBNode *join_nodes(RBTree *tree, RBNode *T1, RBNode *T2, RBNode *k)
{
    int h1 = bh(tree, T1);
    int h2 = bh(tree, T2);
    RBNode *root;

    if (h1 == h2) {
        /* Одинаковая чёрная высота — создаём новый чёрный корень */
        k->color  = RBN_BLACK;
        k->parent = tree->nil;
        k->left   = T1;
        k->right  = T2;
        if (T1 != tree->nil) {
            T1->parent = k;
        }
        if (T2 != tree->nil) {
            T2->parent = k;
        }
        return k;
    }

    if (h1 > h2) {
        root = join_to_right(tree, T1, T2, k);
    } else {
        root = join_to_left(tree, T1, T2, k);
    }

    root->color  = RBN_BLACK;
    root->parent = tree->nil;
    return root;
}

/* Рекурсивное разделение */
static void split_nodes(RBTree *tree, RBNode *T, const void *key,
                        RBNode **L, RBNode **R)
{
    RBNode *Lc, *Rc;
    RBNode *Lp, *Rp;

    if (T == tree->nil) {
        *L = tree->nil;
        *R = tree->nil;
        return;
    }

    Lc = T->left;
    Rc = T->right;

    if (tree->cmp_fn(key, T->key) < 0) {
        if (Rc != tree->nil) {
            Rc->color = RBN_BLACK;
        }
        split_nodes(tree, Lc, key, &Lp, &Rp);
        *L = Lp;
        *R = join_nodes(tree, Rp, Rc, T);
    } else {
        if (Lc != tree->nil) {
            Lc->color = RBN_BLACK;
        }
        split_nodes(tree, Rc, key, &Lp, &Rp);
        *L = join_nodes(tree, Lc, Lp, T);
        *R = Rp;
    }
}

/* Подсчёт узлов в поддереве (для split) */
static unsigned int count_nodes(RBTree *tree, RBNode *node)
{
    if (node == tree->nil) {
        return 0;
    }
    return 1 + count_nodes(tree, node->left)
             + count_nodes(tree, node->right);
}

/* Переназначение nil-узлов при rehome (для join/split) */
static void rehome_subtree(RBNode *old_nil, RBNode *new_nil,
                           RBNode *node, RBNode *parent)
{
    if (node == old_nil) {
        return;
    }
    node->parent = parent;

    if (node->left == old_nil) {
        node->left = new_nil;
    } else {
        rehome_subtree(old_nil, new_nil, node->left, node);
    }

    if (node->right == old_nil) {
        node->right = new_nil;
    } else {
        rehome_subtree(old_nil, new_nil, node->right, node);
    }
}

/* Рекурсивное освобождение поддерева */
static void free_subtree(RBTree *tree, RBNode *node)
{
    if (node == tree->nil) {
        return;
    }

    free_subtree(tree, node->left);
    free_subtree(tree, node->right);

    if (node->key != NULL) {
        tree->key_free_fn(node->key);
    }
    if (node->value != NULL) {
        tree->value_free_fn(node->value);
    }
    free(node);
}

/* Поиск узла по ключу */
static RBNode *find_node(RBTree *tree, const void *key)
{
    RBNode *x = tree->root;
    int cmp;

    while (x != tree->nil) {
        cmp = tree->cmp_fn(key, x->key);
        if (cmp == 0) {
            return x;
        }
        x = (cmp < 0) ? x->left : x->right;
    }
    return NULL;
}

RBTree *create_rbtree(rb_cmp_fn cmp,
                      rb_free_fn free_key,
                      rb_free_fn free_value)
{
    RBTree *tree;
    RBNode *nil;

    if (cmp == NULL || free_key == NULL || free_value == NULL) {
        return NULL;
    }

    tree = (RBTree *)malloc(sizeof(RBTree));
    if (tree == NULL) {
        return NULL;
    }

    nil = (RBNode *)malloc(sizeof(RBNode));
    if (nil == NULL) {
        free(tree);
        return NULL;
    }

    /* Sentinel (nil) всегда чёрный */
    nil->key    = NULL;
    nil->value  = NULL;
    nil->color  = RBN_BLACK;
    nil->parent = nil;
    nil->left   = nil;
    nil->right  = nil;

    tree->nil            = nil;
    tree->root           = nil;
    tree->size           = 0;
    tree->cmp_fn         = cmp;
    tree->key_free_fn    = free_key;
    tree->value_free_fn  = free_value;

    return tree;
}

void destroy_rbtree(RBTree *tree)
{
    if (tree == NULL) {
        return;
    }
    free_subtree(tree, tree->root);
    free(tree->nil);
    free(tree);
}

int rbtree_insert(RBTree *tree, void *key, void *value)
{
    RBNode *t;
    RBNode *p;
    RBNode *q;
    int c;

    if (tree == NULL || key == NULL) {
        return 0;
    }

    t = create_node(tree, RBN_RED, key, value);
    if (t == NULL) {
        return 0;
    }

    if (tree->root == tree->nil) {
        /* Первая вставка — корень чёрный */
        tree->root = t;
        t->parent = tree->nil;
    } else {
        p = tree->root;
        q = tree->nil;

        /* Поиск места вставки */
        while (p != tree->nil) {
            q = p;
            c = tree->cmp_fn(p->key, t->key);
            if (c < 0) {
                p = p->right;
            } else if (c > 0) {
                p = p->left;
            } else {
                /* Ключ уже существует — обновляем значение */
                tree->value_free_fn(p->value);
                p->value = t->value;
                tree->key_free_fn(t->key);   /* новый ключ не нужен */
                free(t);
                return 1;
            }
        }

        t->parent = q;

        if (tree->cmp_fn(q->key, t->key) < 0) {
            q->right = t;
        } else {
            q->left = t;
        }
    }

    tree->size++;
    insert_fixup(tree, &tree->root, t);
    return 1;
}

int rbtree_remove(RBTree *tree, const void *key)
{
    RBNode *p;
    RBNode *y;
    RBNode *q;
    RBNodeColor y_color;
    int c;

    if (tree == NULL || key == NULL) {
        return 0;
    }

    /* Поиск узла для удаления */
    p = tree->root;
    while (p != tree->nil) {
        c = tree->cmp_fn(p->key, key);
        if (c == 0) {
            break;
        } else if (c < 0) {
            p = p->right;
        } else {
            p = p->left;
        }
    }
    if (p == tree->nil) {
        return 0;
    }

    /* Определяем узел y, который реально будет удалён */
    if (p->left == tree->nil && p->right == tree->nil) {
        y = p;
        q = tree->nil;
    } else if (p->left == tree->nil || p->right == tree->nil) {
        y = p;
        q = (p->left != tree->nil) ? p->left : p->right;
    } else {
        /* Два потомка — ищем successor (минимальный в правом поддереве) */
        y = p->right;
        while (y->left != tree->nil) {
            y = y->left;
        }
        q = y->right;
    }
    y_color = y->color;

    /* Переподвешиваем q на место y */
    q->parent = y->parent;
    if (y->parent == tree->nil) {
        tree->root = q;
    } else if (y == y->parent->left) {
        y->parent->left = q;
    } else {
        y->parent->right = q;
    }

    if (y != p) {
        /* Копируем данные successorа в p, освобождаем старые данные p */
        tree->key_free_fn(p->key);
        tree->value_free_fn(p->value);
        p->key   = y->key;
        p->value = y->value;
    } else {
        /* Освобождаем данные y */
        tree->key_free_fn(y->key);
        tree->value_free_fn(y->value);
    }

    free(y);
    tree->size--;

    /* Восстановление, если удалили чёрный узел */
    if (y_color == RBN_BLACK) {
        delete_fixup(tree, q);
    }

    if (tree->root != tree->nil) {
        tree->root->color = RBN_BLACK;
    }

    /* Восстанавливаем корректность sentinel */
    tree->nil->color  = RBN_BLACK;
    tree->nil->parent = tree->nil;

    return 1;
}

void *rbtree_search(RBTree *tree, const void *key)
{
    RBNode *x;

    if (tree == NULL || key == NULL) {
        return NULL;
    }

    x = find_node(tree, key);
    return (x != NULL) ? x->value : NULL;
}

int does_rbtree_contain(RBTree *tree, const void *key)
{
    if (tree == NULL || key == NULL) {
        return 0;
    }
    return (find_node(tree, key) != NULL) ? 1 : 0;
}

RBNode *rbtree_min(RBTree *tree)
{
    RBNode *x;

    if (tree == NULL || tree->root == tree->nil) {
        return NULL;
    }

    x = tree->root;
    while (x->left != tree->nil) {
        x = x->left;
    }
    return x;
}

RBNode *rbtree_max(RBTree *tree)
{
    RBNode *x;

    if (tree == NULL || tree->root == tree->nil) {
        return NULL;
    }

    x = tree->root;
    while (x->right != tree->nil) {
        x = x->right;
    }
    return x;
}

void clear_rbtree(RBTree *tree)
{
    if (tree == NULL) {
        return;
    }
    free_subtree(tree, tree->root);
    tree->root = tree->nil;
    tree->size = 0;
}

int is_rbtree_empty(RBTree *tree)
{
    if (tree == NULL) {
        return 1;
    }
    return tree->size == 0;
}

unsigned int get_rbtree_size(RBTree *tree)
{
    if (tree == NULL) {
        return 0;
    }
    return tree->size;
}


RBTree *rbtree_join(RBTree *t1, RBTree *t2, void *key, void *value)
{
    RBTree *res;
    RBNode *k;
    RBNode *r1, *r2;
    unsigned int n1, n2;

    if (t1 == NULL || t2 == NULL || key == NULL) {
        return NULL;
    }

    /* Проверка совместимости деревьев */
    if (t1->cmp_fn != t2->cmp_fn ||
        t1->key_free_fn != t2->key_free_fn ||
        t1->value_free_fn != t2->value_free_fn) {
        return NULL;
    }

    res = create_rbtree(t1->cmp_fn, t1->key_free_fn, t1->value_free_fn);
    if (res == NULL) {
        return NULL;
    }

    k = create_node(res, RBN_RED, key, value);
    if (k == NULL) {
        destroy_rbtree(res);
        return NULL;
    }

    n1 = t1->size;
    n2 = t2->size;

    r1 = t1->root;
    r2 = t2->root;

    /* Переназначаем nil-узлы */
    if (r1 == t1->nil) {
        r1 = res->nil;
    } else {
        rehome_subtree(t1->nil, res->nil, r1, res->nil);
    }
    if (r2 == t2->nil) {
        r2 = res->nil;
    } else {
        rehome_subtree(t2->nil, res->nil, r2, res->nil);
    }

    res->root = join_nodes(res, r1, r2, k);
    res->root->parent = res->nil;
    res->root->color  = RBN_BLACK;
    res->size = n1 + n2 + 1;

    /* Оригинальные деревья уничтожаются */
    t1->root = t1->nil;
    t1->size = 0;
    t2->root = t2->nil;
    t2->size = 0;
    destroy_rbtree(t1);
    destroy_rbtree(t2);

    return res;
}

int rbtree_split(RBTree *tree, const void *key,
                 RBTree **out_left, RBTree **out_right)
{
    RBTree *L, *R;
    RBNode *Ln, *Rn;

    if (tree == NULL || key == NULL ||
        out_left == NULL || out_right == NULL) {
        return 0;
    }

    L = create_rbtree(tree->cmp_fn, tree->key_free_fn, tree->value_free_fn);
    if (L == NULL) {
        return 0;
    }
    R = create_rbtree(tree->cmp_fn, tree->key_free_fn, tree->value_free_fn);
    if (R == NULL) {
        destroy_rbtree(L);
        return 0;
    }

    split_nodes(tree, tree->root, key, &Ln, &Rn);

    if (Ln == tree->nil) {
        L->root = L->nil;
    } else {
        rehome_subtree(tree->nil, L->nil, Ln, L->nil);
        L->root = Ln;
        L->root->color = RBN_BLACK;
    }
    if (Rn == tree->nil) {
        R->root = R->nil;
    } else {
        rehome_subtree(tree->nil, R->nil, Rn, R->nil);
        R->root = Rn;
        R->root->color = RBN_BLACK;
    }

    L->size = count_nodes(L, L->root);
    R->size = count_nodes(R, R->root);

    /* Оригинальное дерево очищено */
    tree->root = tree->nil;
    tree->size = 0;
    destroy_rbtree(tree);

    *out_left  = L;
    *out_right = R;
    return 1;
}


RBTreeIterator create_rbtree_iterator(RBTree *tree)
{
    RBTreeIterator iter;

    iter.tree = tree;
    iter.stack_top = 0;
    iter.current = NULL;

    /* Инициализируем стек для итеративного inorder */
    if (tree != NULL && tree->root != tree->nil) {
        RBNode *node = tree->root;
        while (node != tree->nil) {
            iter.stack[iter.stack_top++] = node;
            node = node->left;
        }
    }
    return iter;
}

int rbtree_iterator_next(RBTreeIterator *iter, RBNode **node, void **key, void **value)
{
    if (iter == NULL || iter->tree == NULL || iter->stack_top <= 0) {
        return 0;
    }

    /* Берём верхний элемент стека */
    RBNode *curr = iter->stack[--iter->stack_top];
    if (node) *node = curr;
    if (key) *key = curr->key;
    if (value) *value = curr->value;

    /* Добавляем правое поддерево */
    RBNode *right = curr->right;
    while (right != iter->tree->nil) {
        iter->stack[iter->stack_top++] = right;
        right = right->left;
    }

    return 1;
}

/* Рекурсивный inorder для отладки */
void rbtree_inorder(RBTree *tree, rb_visit_fn visit, void *user_data)
{
    if (tree == NULL || visit == NULL) return;

    /* Используем итератор */
    RBTreeIterator iter = create_rbtree_iterator(tree);
    RBNode *node;
    while (rbtree_iterator_next(&iter, &node, NULL, NULL)) {
        visit(node, user_data);
    }
}
