#include <stdio.h>

unsigned int route[100][100];
int k;

void printPath(int[], int);
void printSpace(int);

void dijkstra(int n, int source)
{
    int fin[n];
    unsigned int cost[n], p[n];
    for (int i=0; i<n; i++) {
        fin[i] = 0;
        cost[i] = route[source][i];
        p[i] = source;
    }
    fin[source] = 1;
    cost[source] = 0;
    p[source] = -1;
    for (int i=0; i<n-1; i++) {
        unsigned int min_cost = -1;
        int top = -1;
        for (int j=0; j<n; j++) {
            if (!fin[j] && cost[j] < min_cost) {
                min_cost = cost[j];
                top = j;
            }
        }
        fin[top] = 1;
        for (int j=0; j<n; j++) {
            if (!fin[j] && min_cost < -1 && route[top][j] < -1 && min_cost + route[top][j] < cost[j]) {
                cost[j] = min_cost + route[top][j];
                p[j] = top;
            }
        }
    }
    printf("\nPath to all nodes from Node %d\n", source+1);
    for (int i=0; i<n; i++) {
        k = 0;
        printPath(p, i);
        if (i == source) {
            printf("%d->", source+1);
            k += 3;
        }
        printf("\b\b");
        printSpace(3*n-k+2);
        printf("%d\n", cost[i]);
    }
}


int main()
{
    int n, m;
    scanf("%d %d", &n, &m);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            route[i][j] = -1;
    for (int i=0; i<m; i++) {
        int source, dest, cost;
        scanf("%d %d %d", &source, &dest, &cost);
        route[source-1][dest-1] = route[dest-1][source-1] = cost;
    }


    for (int i=0; i<n; i++) {
        dijkstra(n, i);
    }
}


void printPath(int p[], int j)
{
    k+=3;
    if (p[j] == - 1) {
        printf("%d->", j+1);
        return;
    }
    printPath(p, p[j]);
    printf("%d->", j+1);
}

void printSpace(int n)
{
    for (int i=0; i<n; i++)
        printf(" ");
}