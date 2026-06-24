#ifndef RB_TREE_H
#define RB_TREE_H

/* Цвета узлов красно-черного дерева */
typedef enum {
    RBN_RED,
    RBN_BLACK
} RBNodeColor;

/* Узел красно-черного дерева */
typedef struct RBNode {
    void *key;                  /* Ключ узла (владеет дерево) */
    void *value;                /* Значение узла (владеет дерево) */
    RBNodeColor color;          /* Цвет узла */
    struct RBNode *parent;      /* Родительский узел */
    struct RBNode *left;        /* Левый потомок */
    struct RBNode *right;       /* Правый потомок */
} RBNode;

/* Типы функций для дерева */
typedef int   (*rb_cmp_fn)(const void *a, const void *b);   /* Сравнение ключей: <0, 0, >0 */
typedef void  (*rb_free_fn)(void *a);                       /* Освобождение ключа или значения */

/* Структура красно-черного дерева */
typedef struct {
    RBNode *root;               /* Корень дерева (или nil если пусто) */
    RBNode *nil;                /* Sentinel-узел (чёрный, parent=self, left=right=self) */
    unsigned int size;          /* Количество элементов */
    rb_cmp_fn cmp_fn;           /* Функция сравнения ключей */
    rb_free_fn key_free_fn;     /* Функция освобождения ключей */
    rb_free_fn value_free_fn;   /* Функция освобождения значений */
} RBTree;

/* Итератор для inorder-обхода дерева (для тестирования и использования) */
typedef struct {
    RBTree *tree;
    RBNode *stack[64];          /* Стек для итеративного inorder (достаточно для большинства случаев) */
    int stack_top;
    RBNode *current;
} RBTreeIterator;

/* Создание итератора inorder обхода */
RBTreeIterator create_rbtree_iterator(RBTree *tree);

/* Получение следующего узла в inorder
 *
 * Возвращает 1 если узел получен, 0 если обход завершён
 * node и key/value можно использовать пока не изменено дерево
 */
int rbtree_iterator_next(RBTreeIterator *iter, RBNode **node, void **key, void **value);

/**
 * Создание нового красно-черного дерева
 *
 * cmp - функция сравнения ключей (обязательна, аналог strcmp)
 * free_key - функция освобождения памяти ключа (обязательна)
 * free_value - функция освобождения памяти значения (обязательна)
 *
 * Возвращает указатель на дерево или NULL при ошибке.
 *
 * Дерево владеет всеми ключами и значениями.
 * Память освобождается через destroy_rbtree().
 */
RBTree *create_rbtree(rb_cmp_fn cmp,
                      rb_free_fn free_key,
                      rb_free_fn free_value);

/**
 * Уничтожение дерева
 *
 * Полностью освобождает все узлы, ключи и значения.
 * После вызова указатель tree становится недействительным.
 */
void destroy_rbtree(RBTree *tree);

/**
 * Вставка пары ключ-значение
 *
 * Если ключ уже существует — обновляет значение (старое значение освобождается).
 * Если нет — вставляет новый узел с соблюдением свойств красно-черного дерева.
 *
 * Владение key и value передаётся дереву.
 *
 * tree - дерево
 * key - ключ (не NULL)
 * value - значение (может быть NULL)
 *
 * Возвращает 1 при успехе, 0 при ошибке выделения памяти
 */
int rbtree_insert(RBTree *tree, void *key, void *value);

/**
 * Удаление элемента по ключу
 *
 * Если найден — удаляет узел, освобождает ключ и значение, восстанавливает свойства RB-дерева.
 *
 * tree - дерево
 * key - ключ для удаления
 *
 * Возвращает 1 если элемент был удалён, 0 если ключ не найден
 */
int rbtree_remove(RBTree *tree, const void *key);

/**
 * Поиск значения по ключу
 *
 * tree - дерево
 * key - искомый ключ
 *
 * Возвращает указатель на значение или NULL если ключ не найден.
 * Указатель принадлежит дереву — не освобождайте его.
 */
void *rbtree_search(RBTree *tree, const void *key);

/**
 * Проверка наличия ключа
 *
 * tree - дерево
 * key - ключ
 *
 * Возвращает 1 если ключ существует, 0 иначе
 */
int does_rbtree_contain(RBTree *tree, const void *key);

/**
 * Объединение двух деревьев (join)
 *
 * Создаёт новое дерево, содержащее все элементы t1 + {key,value} + t2.
 * Предполагается, что все ключи в t1 < key < все ключи в t2.
 * t1 и t2 уничтожаются внутри функции (владение передаётся).
 *
 * t1, t2 - деревья (должны иметь одинаковые cmp/free функции)
 * key, value - разделительный ключ-значение
 *
 * Возвращает новое объединённое дерево или NULL при ошибке
 */
RBTree *rbtree_join(RBTree *t1, RBTree *t2, void *key, void *value);

/**
 * Разделение дерева по ключу (split)
 *
 * Разделяет tree на два: left (ключи < key) и right (ключи >= key).
 * Оригинальное дерево уничтожается.
 *
 * tree - исходное дерево
 * key - ключ разделения
 * out_left, out_right - указатели для результата
 *
 * Возвращает 1 при успехе
 */
int rbtree_split(RBTree *tree, const void *key,
                 RBTree **out_left, RBTree **out_right);

/**
 * Возвращает узел с минимальным ключом (самый левый)
 */
RBNode *rbtree_min(RBTree *tree);

/**
 * Возвращает узел с максимальным ключом (самый правый)
 */
RBNode *rbtree_max(RBTree *tree);

/**
 * Очистка дерева (удаляет все элементы, структура остаётся)
 */
void clear_rbtree(RBTree *tree);

/**
 * Проверка на пустоту
 */
int is_rbtree_empty(RBTree *tree);

/**
 * Получение количества элементов
 */
unsigned int get_rbtree_size(RBTree *tree);

/* Вспомогательная функция inorder обхода (для отладки и main) */
typedef void (*rb_visit_fn)(RBNode *node, void *user_data);
void rbtree_inorder(RBTree *tree, rb_visit_fn visit, void *user_data);

#endif /* RB_TREE_H */
