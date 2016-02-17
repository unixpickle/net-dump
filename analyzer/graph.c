#include "graph.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static double * scale_graph(double * values, int pointCount, int cols);
static double max_value(double * values, int count);

void print_graph(double * values, int pointCount) {
  print_graph_at_size(values, pointCount, DEFAULT_GRAPH_WIDTH, DEFAULT_GRAPH_HEIGHT);
}

void print_graph_at_size(double * values, int pointCount, int cols, int rows) {
  double * columns = scale_graph(values, pointCount, cols);
  double maxValue = max_value(columns, cols);

  int row, col;
  for (row = rows; row > 0; --row) {
    for (col = 0; col < cols; ++col) {
      int height = (int)lround((double)cols * columns[col] / maxValue);
      if (height >= row) {
        printf("#");
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }

  free(columns);
}

static double * scale_graph(double * values, int pointCount, int cols) {
  double * columnValues = (double *)malloc(sizeof(double) * cols);
  double columnWidth = (double)pointCount / (double)cols;

  int i;
  for (i = 0; i < cols; ++i) {
    double columnLeft = columnWidth * (double)i;
    double columnValue = 0;

    int pointIdx;
    for (pointIdx = 0; pointIdx < pointCount; ++pointIdx) {
      double pointLeft = (double)pointIdx;
      double pointRight = pointLeft + 1;
      if (pointRight <= columnLeft || pointLeft >= columnLeft+columnWidth) {
        continue;
      }
      double amount = 1;
      if (pointLeft < columnLeft) {
        amount -= columnLeft - pointLeft;
      }
      if (pointRight > columnLeft+columnWidth) {
        amount -= pointRight - (columnLeft + columnWidth);
      }
      columnValue += amount * values[pointIdx];
    }
    columnValue /= columnWidth;
    columnValues[i] = columnValue;
  }

  return columnValues;
}

static double max_value(double * values, int count) {
  if (count == 0) {
    return 0;
  }

  double value = values[0];

  int i;
  for (i = 1; i < count; ++i) {
    if (values[i] > value) {
      value = values[i];
    }
  }

  return value;
}
