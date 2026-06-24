#include <stdlib.h>
#include "rbmap.h"
#include "rbtree.h"

/* Создание map */
Map *create_map(rb_cmp_fn cmp,
                rb_free_fn free_key,
                rb_free_fn free_value)
{
    Map *map;

    map = (Map *)malloc(sizeof(Map));
    if (map == NULL) {
        return NULL;
    }

    map->tree = create_rbtree(cmp, free_key, free_value);
    if (map->tree == NULL) {
        free(map);
        return NULL;
    }

    return map;
}

/* Уничтожение map */
void destroy_map(Map *map)
{
    if (map == NULL) {
        return;
    }
    destroy_rbtree(map->tree);
    free(map);
}

/* Вставка или обновление */
int map_put(Map *map, void *key, void *value)
{
    if (map == NULL || key == NULL) {
        return 0;
    }

    return rbtree_insert(map->tree, key, value);
}

/* Получение значения */
void *map_get(Map *map, const void *key)
{
    if (map == NULL || key == NULL) {
        return NULL;
    }
    return rbtree_search(map->tree, key);
}

/* Удаление по ключу */
int map_remove(Map *map, const void *key)
{
    if (map == NULL || key == NULL) {
        return 0;
    }
    return rbtree_remove(map->tree, key);
}

/* Проверка наличия ключа */
int does_map_contain(Map *map, const void *key)
{
    if (map == NULL || key == NULL) {
        return 0;
    }
    return does_rbtree_contain(map->tree, key);
}

/* Размер */
unsigned int get_map_size(Map *map)
{
    if (map == NULL) {
        return 0;
    }
    return get_rbtree_size(map->tree);
}

/* Пустой ли map */
int is_map_empty(Map *map)
{
    if (map == NULL) {
        return 1;
    }
    return is_rbtree_empty(map->tree);
}

/* Очистка */
void clear_map(Map *map)
{
    if (map == NULL) {
        return;
    }
    clear_rbtree(map->tree);
}

/* Минимальный ключ */
RBNode *map_min(Map *map)
{
    if (map == NULL) {
        return NULL;
    }
    return rbtree_min(map->tree);
}

/* Максимальный ключ */
RBNode *map_max(Map *map)
{
    if (map == NULL) {
        return NULL;
    }
    return rbtree_max(map->tree);
}
