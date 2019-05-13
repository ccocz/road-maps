#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

bool badName(const char *city);
City *cityExists(Map *map, const char *city);
Road *isConnected(City *city1, City *city2);
City *addCity(Map *map, const char *city);
void connectCities(Map *map, City *city1, City *city2,
                   unsigned length, int builtYear);

struct Command {
  char *line;
  size_t length;
  int lineNumber;
  Map *map;
};

typedef struct Command Command;

bool endOfComponent(char c) {
  return c == ';' || c == '\0' || c == '\n';
}

char *nextComponent(size_t *to, size_t from, char *line) {
  char *new = NULL;
  size_t total = 0;
  size_t i;
  for (i = from; i < strlen(line) && !endOfComponent(line[i]); i++) {
    total++;
  }
  new = malloc(total + 1);
  for (i = from; i < strlen(line) && !endOfComponent(line[i]); i++) {
    new[i - from] = line[i];
  }
  *to = i;
  new[total] = '\0';
  return new;
}

bool isInt(const char *s) {
  size_t i = (s[0] == '-');
  size_t n = strlen(s);
  size_t nonZero = n;
  for (; i < n; i++) {
    if (!(s[i] >= '0' && s[i] <= '9')) {
      return false;
    }
    if (s[i] != '0' && nonZero == n) {
      nonZero = i;
    }
  }
  if (nonZero == n) {
    return false;
  }
  const char *maxi = "2147483647\0";
  n -= nonZero;
  if (n > strlen(maxi)) {
    return false;
  } else if (n < strlen(maxi)) {
    return true;
  } else {
    if (strcmp(&s[nonZero], maxi) > 0) {
      return false;
    } else {
      return true;
    }
  }
}

bool isUInt(const char *s) {
  size_t n = strlen(s);
  size_t nonZero = n;
  for (size_t i = 0; i < n; i++) {
    if (!(s[i] >= '0' && s[i] <= '9')) {
      return false;
    }
    if (s[i] != '0' && nonZero == n) {
      nonZero = i;
    }
  }
  if (nonZero == n) {
    return false;
  }
  const char *maxi = "4294967295\0";
  n -= nonZero;
  if (n > strlen(maxi)) {
    return false;
  } else if (n < strlen(maxi)) {
    return true;
  } else {
    if (strcmp(&s[nonZero], maxi) > 0) {
      return false;
    } else {
      return true;
    }
  }
}

void errorOnLine(int lineNumber) {
  fprintf(stderr, "ERROR %d\n", lineNumber);
}

void checkAddRoad(Command command) {
  size_t lastPosition = strlen("addRoad;\0");
  char *city1 = nextComponent(&lastPosition, lastPosition, command.line);
  if (command.line[lastPosition] != ';') {
    free(city1);
    errorOnLine(command.lineNumber);
    return;
  }
  char *city2 = nextComponent(&lastPosition, ++lastPosition, command.line);
  if (command.line[lastPosition] != ';') {
    free(city1);
    free(city2);
    errorOnLine(command.lineNumber);
    return;
  }
  char *length = nextComponent(&lastPosition, ++lastPosition, command.line);
  if (command.line[lastPosition] != ';' || !isUInt(length)) {
    free(city1);
    free(city2);
    free(length);
    errorOnLine(command.lineNumber);
    return;
  }
  char *builtYear = nextComponent(&lastPosition, ++lastPosition, command.line);
  if (command.line[lastPosition] != '\n' || !isInt(builtYear)) {
    free(city1);
    free(city2);
    free(length);
    free(builtYear);
    errorOnLine(command.lineNumber);
    return;
  }
  unsigned roadLength = strtol(length, NULL, 10);
  int roadYear = strtol(builtYear, NULL, 10);
  if (!addRoad(command.map, city1, city2, roadLength, roadYear)) {
    errorOnLine(command.lineNumber);
  }
  free(city1);
  free(city2);
  free(length);
  free(builtYear);
}

void checkRepairRoad(Command command) {
  size_t lastPosition = strlen("repairRoad;\0");
  char *city1 = nextComponent(&lastPosition, lastPosition, command.line);
  if (command.line[lastPosition] != ';') {
    free(city1);
    errorOnLine(command.lineNumber);
    return;
  }
  char *city2 = nextComponent(&lastPosition, ++lastPosition, command.line);
  if (command.line[lastPosition] != ';') {
    free(city1);
    free(city2);
    errorOnLine(command.lineNumber);
    return;
  }
  char *repairYear = nextComponent(&lastPosition, ++lastPosition, command.line);
  if (command.line[lastPosition] != '\n' || !isInt(repairYear)) {
    free(city1);
    free(city2);
    free(repairYear);
    errorOnLine(command.lineNumber);
    return;
  }
  int roadYear = strtol(repairYear, NULL, 10);
  if (!repairRoad(command.map, city1, city2, roadYear)) {
    errorOnLine(command.lineNumber);
  }
  free(city1);
  free(city2);
  free(repairYear);
}

void checkDescription(Command command) {
  size_t lastPosition = strlen("getRouteDescription;\0");
  char *routeId = nextComponent(&lastPosition, lastPosition, command.line);
  if (command.line[lastPosition] != '\n' || !isUInt(routeId)) {
    free(routeId);
    errorOnLine(command.lineNumber);
    return;
  }
  unsigned id = strtol(routeId, NULL, 10);
  const char *result = getRouteDescription(command.map, id);
  if (result && strlen(result)) {
    puts(result);
  }
  free(routeId);
  free((void *)result);
}

bool checkRouteFirst(Command command) {
  size_t lastPosition = 0;
  free(nextComponent(&lastPosition, 0, command.line));
  char *lastCity = nextComponent(&lastPosition, ++lastPosition, command.line);
  if (command.line[lastPosition] != ';') {
    free(lastCity);
    return false;
  }
  bool stop = false;
  char *use;
  unsigned length;
  int year;
  City *left = cityExists(command.map, lastCity), *right;
  Road *road;
  free(lastCity);
  while (!stop) {
    if (lastPosition + 1 > command.length - 1) {
      return false;
    }
    use = nextComponent(&lastPosition, ++lastPosition, command.line);
    if (command.line[lastPosition] != ';' || !isUInt(use)) {
      free(use);
      return false;
    }
    length = strtol(use, NULL, 10);
    free(use);
    use = nextComponent(&lastPosition, ++lastPosition, command.line);
    if (command.line[lastPosition] != ';' || !isInt(use)) {
      free(use);
      return false;
    }
    year = strtol(use, NULL, 10);
    free(use);
    use = nextComponent(&lastPosition, ++lastPosition, command.line);
    if (command.line[lastPosition] == '\n') {
      stop = true;
    } else if (command.line[lastPosition] != ';') {
      free(use);
      return false;
    }
    free(use);
    right = cityExists(command.map, use);
    if (left && right) {
      road = isConnected(left, right);
      if (road->length != length || road->year > year) {
        return false;
      }
    }
    left = right;
  }
  return true;
}

void makeNewRoute(Command command) {
  size_t lastPosition = 0;
  char *routeId = nextComponent(&lastPosition, 0, command.line);
  char *lastCity = nextComponent(&lastPosition, ++lastPosition, command.line);
  bool stop = false;
  char *use;
  unsigned length;
  int year;
  City *left = cityExists(command.map, lastCity), *right;
  if (!left) {
    left = addCity(command.map, lastCity);
  }
  free(lastCity);
  Route *newRoute = malloc(sizeof(Route));
  Edges *useEdges;
  newRoute->start = left;
  newRoute->edges = NULL;
  while (!stop) {
    puts(left->name);
    use = nextComponent(&lastPosition, ++lastPosition, command.line);
    length = strtol(use, NULL, 10);
    free(use);
    use = nextComponent(&lastPosition, ++lastPosition, command.line);
    year = strtol(use, NULL, 10);
    free(use);
    use = nextComponent(&lastPosition, ++lastPosition, command.line);
    if (command.line[lastPosition] == '\n') {
      stop = true;
    }
    right = cityExists(command.map, use);
    if (!right) {
      right = addCity(command.map, use);
      if (stop) {
        newRoute->end = right;
      }
    }
    free(use);
    if (!isConnected(left, right)) {
      connectCities(command.map, left, right, length, year);
    }
    useEdges = malloc(sizeof(Edges));
    useEdges->road = isConnected(left, right);
    useEdges->next = NULL;
    useEdges->prev = newRoute->edges;
    if (newRoute->edges) {
      newRoute->edges->next = useEdges;
    }
    newRoute->edges = useEdges;
    left = right;
  }
  unsigned id = strtol(routeId, NULL, 0);
  while (newRoute->edges->prev) {
    newRoute->edges = newRoute->edges->prev;
  }
  command.map->routes[id] = newRoute;
  free(routeId);
}

void checkNewRoute(Command command) {
  if (!checkRouteFirst(command)) {
    errorOnLine(command.lineNumber);
    return;
  }
  makeNewRoute(command);
}

void switchCommand(Command command) {
  if (command.line[0] == '#' || command.line[0] == '\n') {
    return;
  }
  size_t start = 0;
  char *beginWith = nextComponent(&start, 0, command.line);
  if (command.line[start] != ';') {
    fprintf(stderr, "ERROR %d\n", command.lineNumber);
  } else if (!strcmp(beginWith, "addRoad\0")) {
    checkAddRoad(command);
  } else if (!strcmp(beginWith, "repairRoad\0")) {
    checkRepairRoad(command);
  } else if (!strcmp(beginWith, "getRouteDescription\0")) {
    checkDescription(command);
  } else {
    if (isUInt(beginWith)) {
      checkNewRoute(command);
    } else {
      fprintf(stderr, "ERROR %d\n", command.lineNumber);
    }
  }
  free(beginWith);
}

void start() {
  Command command;
  command.line = NULL;
  command.length = 0;
  command.lineNumber = 0;
  command.map = newMap();
  while (getline(&command.line, &command.length, stdin) != -1) {
    command.lineNumber++;
    switchCommand(command);
    free(command.line);
    command.line = NULL;
    command.length = 0;
  }
  deleteMap(command.map);
  free(command.line);
}

int main(void) {
  start();
  return 0;
}