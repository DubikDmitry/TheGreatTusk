#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lodepng.h"

// Структура для ребра графа
typedef struct {
    int u, v;
    float weight;
} Edge;

// Структура для подмножества в алгоритме Краскала
typedef struct {
    int parent;
    int rank;
} Subset;

// Функция для сравнения рёбер (для qsort)
int compareEdges(const void* a, const void* b) {
    Edge* edgeA = (Edge*)a;
    Edge* edgeB = (Edge*)b;
    return (edgeA->weight > edgeB->weight) ? 1 : -1;
}

// Функция для нахождения корня подмножества
int find(Subset subsets[], int i) {
    if (subsets[i].parent != i)
        subsets[i].parent = find(subsets, subsets[i].parent);
    return subsets[i].parent;
}

// Функция для объединения двух подмножеств
void unionSets(Subset subsets[], int x, int y) {
    int xroot = find(subsets, x);
    int yroot = find(subsets, y);

    if (subsets[xroot].rank < subsets[yroot].rank)
        subsets[xroot].parent = yroot;
    else if (subsets[xroot].rank > subsets[yroot].rank)
        subsets[yroot].parent = xroot;
    else {
        subsets[yroot].parent = xroot;
        subsets[xroot].rank++;
    }
}


void kruskalMST(Edge* edges, int numEdges, int numVertices, Edge* result) {
    qsort(edges, numEdges, sizeof(Edge), compareEdges);

    Subset* subsets = (Subset*)malloc(numVertices * sizeof(Subset));
    for (int v = 0; v < numVertices; v++) {
        subsets[v].parent = v;
        subsets[v].rank = 0;
    }

    int e = 0;
    int i = 0;
    while (e < numVertices - 1 && i < numEdges) {
        Edge nextEdge = edges[i++];

        int x = find(subsets, nextEdge.u);
        int y = find(subsets, nextEdge.v);

        if (x != y) {
            result[e++] = nextEdge;
            unionSets(subsets, x, y);
        }
    }

    free(subsets);
}

// Функция для преобразования изображения и сохранения результата
void processImage(const char* inputFilename, const char* outputFilename) {
    unsigned char* image = NULL;
    unsigned width, height;
    unsigned error = lodepng_decode32_file(&image, &width, &height, inputFilename);

    if (error) {
        printf("Ошибка декодирования %s: %s\n", inputFilename, lodepng_error_text(error));
        return;
    }

    int numPixels = width * height;
    int numEdges = (width - 1) * height + (height - 1) * width;
    Edge* edges = (Edge*)malloc(numEdges * sizeof(Edge));
    int edgeIndex = 0;

    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int current = y * width + x;
            unsigned char* pixel = &image[4 * current];
            float intensity = (pixel[0] + pixel[1] + pixel[2]) / 3.0f;

            if (x < width - 1) { // Право
                int right = y * width + (x + 1);
                unsigned char* rightPixel = &image[4 * right];
                float rightIntensity = (rightPixel[0] + rightPixel[1] + rightPixel[2]) / 3.0f;
                edges[edgeIndex].u = current;
                edges[edgeIndex].v = right;
                edges[edgeIndex].weight = fabs(intensity - rightIntensity);
                edgeIndex++;
            }

            if (y < height - 1) { // Низ
                int bottom = (y + 1) * width + x;
                unsigned char* bottomPixel = &image[4 * bottom];
                float bottomIntensity = (bottomPixel[0] + bottomPixel[1] + bottomPixel[2]) / 3.0f;
                edges[edgeIndex].u = current;
                edges[edgeIndex].v = bottom;
                edges[edgeIndex].weight = fabs(intensity - bottomIntensity);
                edgeIndex++;
            }
        }
    }

    Edge* mst = (Edge*)malloc((numPixels - 1) * sizeof(Edge));
    kruskalMST(edges, numEdges, numPixels, mst);

    // Создаём копию изображения для модификации
    unsigned char* newImage = (unsigned char*)malloc(4 * numPixels);
    memcpy(newImage, image, 4 * numPixels);

    // Перекрашиваем пиксели остовного дерева
    for (int i = 0; i < numPixels - 1; i++) {
        int u = mst[i].u;
        int v = mst[i].v;

        // Перекрашиваем оба пикселя ребра
        for (int j = 0; j < 2; j++) {
            int pixelIdx = (j == 0) ? u : v;
            unsigned char* pixel = &newImage[4 * pixelIdx];
            float intensity = (pixel[0] + pixel[1] + pixel[2]) / 3.0f;

            // Чем светлее пиксель, тем он зеленее; чем темнее - тем краснее
            pixel[0] = (unsigned char)(255 - intensity)*0.5; // Красный
            pixel[1] = (unsigned char)(intensity);       // Зелёный
            pixel[2] = 0;                                // Синий (не используется)
            pixel[3] = 255;                              // Альфа (полностью непрозрачный)
        }
    }

    // Сохраняем результат
    lodepng_encode32_file(outputFilename, newImage, width, height);

    free(edges);
    free(mst);
    free(newImage);
    free(image);
}

int main() {
    const char* inputFiles[] = {"Task.png"};
    const char* outputFiles[] = {"New_task.png"};

    for (int i = 0; i < 5; i++) {
        processImage(inputFiles[i], outputFiles[i]);
        printf("Обработано: %s -> %s\n", inputFiles[i], outputFiles[i]);
    }

    return 0;
}