#include <stdio.h>

int main()
{
    int n, m;
    scanf("%d %d", &n, &m);
    unsigned int route[n][n], next_hop[n][n];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            route[i][j] = -1;
    for (int i=0; i<m; i++) {
        int source, dest, cost;
        scanf("%d %d %d", &source, &dest, &cost);
        route[source-1][dest-1] = route[dest-1][source-1] = cost;
    }
    for (int i=0; i<n; i++) {
        route[i][i] = 0;
        for(int j=0; j<n; j++)
            next_hop[i][j] = j;
    }
    int flag;
    do {
        flag = 0;
        for (int i=0; i<n; i++) {
            for (int j=0; j<n; j++) {
                for (int k=0; k<n; k++) {
                    if (route[i][j] > route[i][k] + route[k][j]) {
                        route[i][j] = route[i][k] + route[k][j];
                        next_hop[i][j] = next_hop[i][k];
                        flag = 1;
                    }
                }
            }
        }
    } while (flag);
    for (int i=0; i<n; i++) {
        printf("\nRouting Table at Node %d\n", i+1);
        printf("+-----+-----+-----+\n");
        printf("|Dest |Next |Cost |\n");
        printf("+-----+-----+-----+\n");
        for (int j=0; j<n; j++)
            printf("|%4d |%4d |%4d |\n", j+1, next_hop[i][j]+1, route[i][j]);
        printf("+-----+-----+-----+\n");
    }
}