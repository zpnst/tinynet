#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "src/sys.h"
#include "src/tinynet.h"

void 
floyd_warshall(tinynet_conf_t *net_conf) 
{
    size_t vertex = get_device_count(net_conf);
    
    __uint32_t **dist = (__uint32_t**)panic_alloc(vertex * sizeof(__uint32_t *));
    __int32_t **hops_matrix = (__int32_t**)panic_alloc(vertex * sizeof(__int32_t *));

    for (size_t iter = 0; iter < vertex; iter += 1) {
        dist[iter] = (__uint32_t*)panic_alloc(vertex * sizeof(__uint32_t));
        hops_matrix[iter] = (__int32_t*)panic_alloc(vertex * sizeof(__int32_t));
    }

    for (size_t iter = 0; iter < vertex; iter += 1) {
        for (size_t jter = 0; jter < vertex; jter += 1) {
            dist[iter][jter] = INT_MAX;
            hops_matrix[iter][jter] = -1;
        }
        dist[iter][iter] = 0;
        hops_matrix[iter][iter] = (__int32_t)iter;
    }


    /** Формирование матрицы смежности */
    for (size_t iter = 0; iter < vertex; iter += 1) {
        adjacency_node_t *head = net_conf->net_graph->adjacency_list[iter];
        if (!head) continue;

        adjacency_node_t *next_node = head->next;
        while (next_node) {
            __int32_t jter = get_device_index_by_name(net_conf, next_node->basic_info.dev_name);
            if (jter >= 0 && jter < (__int32_t)vertex) {
                if (dist[iter][jter] > next_node->weight) {
                    dist[iter][jter] = next_node->weight;
                    /** Тривиальный путь от iter к jter при инициализации 
                     *  то есть следующий узел после iter на кратчайшем пути к jter — это jter
                    */
                    hops_matrix[iter][jter] = jter; 
                }
            }
            next_node = next_node->next;
        }
    }

    /** Алгоритм Флойда — Уоршелла */
    for (size_t kter = 0; kter < vertex; kter += 1) {
        for (size_t iter = 0; iter < vertex; iter += 1) {
            if (dist[iter][kter] == INT_MAX) continue; 
            for (size_t jter = 0; jter < vertex; jter += 1) {
                if (dist[kter][jter] == INT_MAX) continue;
                __uint32_t alt = dist[iter][kter] + dist[kter][jter];
                if (alt < dist[iter][jter]) {
                    /** Обновляем вес пути на альтернативный */
                    dist[iter][jter] = alt; 
                    /** Первый шаг на пути от iter к jter такой же, 
                     * как и первый шаг на пути от iter к kter, так
                     * как путь iter -> kter -> jter начинается так же, как путь iter -> kter */
                    hops_matrix[iter][jter] = hops_matrix[iter][kter];
                }
            }
        }
    }
    

    size_t start_hosts = net_conf->net_graph->rc + net_conf->net_graph->sc;
    size_t end_hosts = start_hosts + net_conf->net_graph->hc;

    for (size_t iter = start_hosts; iter < end_hosts; iter += 1) {
        for (size_t jter = start_hosts; jter < end_hosts; jter += 1) {
            if (dist[iter][jter] == INT_MAX) {
                panic("incorrect path");
            } 
        }
    }

    for (size_t iter = 0; iter < vertex; iter += 1) {
        safety_free(dist[iter]);
    }

    safety_free(dist);

    net_conf->hops_matrix = hops_matrix;
}
