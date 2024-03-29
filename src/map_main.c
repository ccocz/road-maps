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
void giveId(Route *route, unsigned routeId);
bool existId(Road *road, unsigned id);

/**
 * @brief Structure to combine command's components
 */
struct Command {
  char *line;     /**<The command itself represented as a string*/
  size_t length;  /**<Length of the command*/
  int lineNumber; /**<Line number in input which command is given*/
  Map *map;       /**<Structure map which is used in all of the commands*/
};
/**
 * @brief Structure to store visited cities for proper route
 */
struct visitedCities {
  City *city;                     /**<Address of the city*/
  struct visitedCities *next;     /**<Next city in the list*/
};

typedef struct Command Command;
typedef struct visitedCities visitedCities;

bool endOfComponent(char c) {
  return c == ';' || c == '\0' || c == '\n';
}

char *nextComponent(size_t *to, size_t from, char *line, size_t length) {
  char *new = NULL;
  size_t total = 0;
  size_t i;
  for (i = from; i < length && !endOfComponent(line[i]); i++) {
    total++;
  }
  new = malloc(total + 1);
  for (i = from; i < length && !endOfComponent(line[i]); i++) {
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
    if (s[0] == '-') {
      maxi = "2147483648\0";
    }
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
    return true;
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
  char *city1 = nextComponent(&lastPosition, lastPosition, command.line,
                              command.length);
  if (!strlen(city1) || command.line[lastPosition] != ';') {
    free(city1);
    errorOnLine(command.lineNumber);
    return;
  }
  char *city2 = nextComponent(&lastPosition, ++lastPosition, command.line,
                              command.length);
  if (!strlen(city2) || command.line[lastPosition] != ';') {
    free(city1);
    free(city2);
    errorOnLine(command.lineNumber);
    return;
  }
  char *length = nextComponent(&lastPosition, ++lastPosition, command.line,
                               command.length);
  if (!strlen(length) || command.line[lastPosition] != ';' || !isUInt(length)) {
    free(city1);
    free(city2);
    free(length);
    errorOnLine(command.lineNumber);
    return;
  }
  char *builtYear = nextComponent(&lastPosition, ++lastPosition, command.line,
                                  command.length);
  if (!strlen(builtYear) || command.line[lastPosition] != '\n' ||
      !isInt(builtYear)) {
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
  char *city1 = nextComponent(&lastPosition, lastPosition, command.line,
                              command.length);
  if (!strlen(city1) || command.line[lastPosition] != ';') {
    free(city1);
    errorOnLine(command.lineNumber);
    return;
  }
  char *city2 = nextComponent(&lastPosition, ++lastPosition, command.line,
                              command.length);
  if (!strlen(city2) || command.line[lastPosition] != ';') {
    free(city1);
    free(city2);
    errorOnLine(command.lineNumber);
    return;
  }
  char *repairYear = nextComponent(&lastPosition, ++lastPosition, command.line,
                                   command.length);
  if (!strlen(repairYear) || command.line[lastPosition] != '\n' || !isInt
      (repairYear)) {
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
  char *routeId = nextComponent(&lastPosition, lastPosition, command.line,
                                command.length);
  if (!strlen(routeId) || command.line[lastPosition] != '\n' || !isUInt
      (routeId)) {
    free(routeId);
    errorOnLine(command.lineNumber);
    return;
  }
  unsigned id = strtol(routeId, NULL, 10);
  const char *result = getRouteDescription(command.map, id);
  if (result) {
    puts(result);
  }
  free(routeId);
  free((void *)result);
}

void addVisited(visitedCities **cities, City *city) {
  visitedCities *use = malloc(sizeof(visitedCities));
  use->city = city;
  use->next = *cities;
  (*cities) = use;
}

void freeVisited(visitedCities *cities) {
  visitedCities *use;
  while (cities) {
    use = cities->next;
    cities->city->allowed = true;
    free(cities);
    cities = use;
  }
}

bool checkRouteFirst(Command command) {
  size_t lastPosition = 0;
  char *routeId = nextComponent(&lastPosition, 0, command.line, command.length);
  unsigned id = strtol(routeId, NULL, 10);
  if (id > 999 || command.map->routes[id]) {
    free(routeId);
    return false;
  }
  free(routeId);
  char *lastCity = nextComponent(&lastPosition, ++lastPosition, command.line,
                                 command.length);
  if (command.line[lastPosition] != ';' || badName(lastCity)) {
    free(lastCity);
    return false;
  }
  bool stop = false;
  char *use;
  unsigned length;
  int year;
  City *left = cityExists(command.map, lastCity), *right;
  if (!left) {
    left = addCity(command.map, lastCity);
  }
  left->allowed = false;
  free(lastCity);
  Road *road;
  visitedCities *cities = NULL;
  addVisited(&cities, left);
  while (!stop) {
    if (lastPosition + 1 > command.length - 1) {
      freeVisited(cities);
      return false;
    }
    use = nextComponent(&lastPosition, ++lastPosition, command.line,
                        command.length);
    if (!strlen(use) || command.line[lastPosition] != ';' || !isUInt(use) ||
        !(unsigned)strtol(use, NULL, 10)) {
      freeVisited(cities);
      free(use);
      return false;
    }
    length = strtol(use, NULL, 10);
    free(use);
    use = nextComponent(&lastPosition, ++lastPosition, command.line,
                        command.length);
    if (!strlen(use) || command.line[lastPosition] != ';' || !isInt(use)) {
      freeVisited(cities);
      free(use);
      return false;
    }
    year = strtol(use, NULL, 10);
    free(use);
    use = nextComponent(&lastPosition, ++lastPosition, command.line,
                        command.length);
    if (badName(use)) {
      freeVisited(cities);
      free(use);
      return false;
    }
    if (command.line[lastPosition] == '\n') {
      stop = true;
    } else if (command.line[lastPosition] != ';') {
      freeVisited(cities);
      free(use);
      return false;
    }
    right = cityExists(command.map, use);
    if (!right) {
      right = addCity(command.map, use);
      right->allowed = false;
      addVisited(&cities, right);
      free(use);
    } else {
      free(use);
      if (!right->allowed) {
        freeVisited(cities);
        return false;
      } else {
        right->allowed = false;
        addVisited(&cities, right);
      }
    }
    road = isConnected(left, right);
    if (road && (road->length != length || road->year > year)) {
      freeVisited(cities);
      return false;
    }
    left = right;
  }
  freeVisited(cities);
  return true;
}

void makeNewRoute(Command command) {
  size_t lastPosition = 0;
  char *routeId = nextComponent(&lastPosition, 0, command.line, command.length);
  char *lastCity = nextComponent(&lastPosition, ++lastPosition, command.line,
                                 command.length);
  bool stop = false;
  char *use;
  unsigned length;
  int year;
  City *left = cityExists(command.map, lastCity), *right = NULL;
  free(lastCity);
  Route *newRoute = malloc(sizeof(Route));
  Edges *useEdges, *starting = NULL;
  Road *useRoad;
  newRoute->start = left;
  newRoute->edges = NULL;
  while (!stop) {
    use = nextComponent(&lastPosition, ++lastPosition, command.line,
                        command.length);
    length = strtol(use, NULL, 10);
    free(use);
    use = nextComponent(&lastPosition, ++lastPosition, command.line,
                        command.length);
    year = strtol(use, NULL, 10);
    free(use);
    use = nextComponent(&lastPosition, ++lastPosition, command.line,
                        command.length);
    if (command.line[lastPosition] == '\n') {
      stop = true;
    }
    right = cityExists(command.map, use);
    if (stop) {
      newRoute->end = right;
    }
    free(use);
    useRoad = isConnected(left, right);
    if (!useRoad) {
      connectCities(command.map, left, right, length, year);
    } else {
      useRoad->year = year;
    }
    useEdges = malloc(sizeof(Edges));
    useEdges->road = isConnected(left, right);
    useEdges->next = NULL;
    useEdges->prev = newRoute->edges;
    if (newRoute->edges) {
      newRoute->edges->next = useEdges;
    } else {
      starting = useEdges;
    }
    newRoute->edges = useEdges;
    left = right;
  }
  unsigned id = strtol(routeId, NULL, 0);
  newRoute->edges = starting;
  Map *map = command.map;
  map->routes[id] = newRoute;
  giveId(newRoute, id);
  free(routeId);
}

void checkNewRoute(Command command) {
  if (!checkRouteFirst(command)) {
    errorOnLine(command.lineNumber);
    return;
  }
  makeNewRoute(command);
}

void checkAddRoute(Command command) {
  size_t lastPosition = strlen("newRoute;\0");
  char *routeId = nextComponent(&lastPosition, lastPosition, command.line,
                                command.length);
  if (!strlen(routeId) || command.line[lastPosition] != ';' || !isUInt
    (routeId)) {
    free(routeId);
    errorOnLine(command.lineNumber);
    return;
  }
  char *city1 = nextComponent(&lastPosition, ++lastPosition, command.line,
                              command.length);
  if (!strlen(city1) || command.line[lastPosition] != ';') {
    free(routeId);
    free(city1);
    errorOnLine(command.lineNumber);
    return;
  }
  char *city2 = nextComponent(&lastPosition, ++lastPosition, command.line,
                              command.length);
  if (!strlen(city2) || command.line[lastPosition] != '\n') {
    free(routeId);
    free(city1);
    free(city2);
    errorOnLine(command.lineNumber);
    return;
  }
  unsigned id = strtol(routeId, NULL, 10);
  if (!newRoute(command.map, id, city1, city2)) {
    errorOnLine(command.lineNumber);
  }
  free(routeId);
  free(city1);
  free(city2);
}

void checkExtendRoute(Command command) {
  size_t lastPosition = strlen("extendRoute;\0");
  char *routeId = nextComponent(&lastPosition, lastPosition, command.line,
                                command.length);
  if (!strlen(routeId) || command.line[lastPosition] != ';' || !isUInt
    (routeId)) {
    free(routeId);
    errorOnLine(command.lineNumber);
    return;
  }
  char *city = nextComponent(&lastPosition, ++lastPosition, command.line,
                             command.length);
  if (!strlen(city) || command.line[lastPosition] != '\n') {
    free(routeId);
    free(city);
    errorOnLine(command.lineNumber);
    return;
  }
  unsigned id = strtol(routeId, NULL, 10);
  if (!extendRoute(command.map, id, city)) {
    errorOnLine(command.lineNumber);
  }
  free(routeId);
  free(city);
}

void checkRemoveRoad(Command command) {
  size_t lastPosition = strlen("removeRoad;\0");
  char *city1 = nextComponent(&lastPosition, lastPosition, command.line,
                              command.length);
  if (!strlen(city1) || command.line[lastPosition] != ';') {
    free(city1);
    errorOnLine(command.lineNumber);
    return;
  }
  char *city2 = nextComponent(&lastPosition, ++lastPosition, command.line,
                              command.length);
  if (!strlen(city2) || command.line[lastPosition] != '\n') {
    free(city1);
    free(city2);
    errorOnLine(command.lineNumber);
    return;
  }
  if (!removeRoad(command.map, city1, city2)) {
    errorOnLine(command.lineNumber);
  }
  free(city1);
  free(city2);
}

void checkRemoveRoute(Command command) {
  size_t lastPosition = strlen("removeRoute;\0");
  char *routeId = nextComponent(&lastPosition, lastPosition, command.line,
                                command.length);
  if (!strlen(routeId) || command.line[lastPosition] != '\n' || !isUInt
    (routeId)) {
    free(routeId);
    errorOnLine(command.lineNumber);
    return;
  }
  unsigned id = strtol(routeId, NULL, 10);
  if (!removeRoute(command.map, id)) {
    errorOnLine(command.lineNumber);
  }
  free(routeId);
}

void switchCommand(Command command) {
  if (command.line[0] == '#' || command.line[0] == '\n') {
    return;
  }
  size_t start = 0;
  char *beginWith = nextComponent(&start, 0, command.line, command.length);
  if (command.line[start] != ';') {
    fprintf(stderr, "ERROR %d\n", command.lineNumber);
  } else if (!strcmp(beginWith, "addRoad\0")) {
    checkAddRoad(command);
  } else if (!strcmp(beginWith, "repairRoad\0")) {
    checkRepairRoad(command);
  } else if (!strcmp(beginWith, "getRouteDescription\0")) {
    checkDescription(command);
  } else if (!strcmp(beginWith, "newRoute\0")) {
    checkAddRoute(command);
  } else if (!strcmp(beginWith, "extendRoute\0")) {
    checkExtendRoute(command);
  } else if (!strcmp(beginWith, "removeRoad\0")) {
    checkRemoveRoad(command);
  } else if (!strcmp(beginWith, "removeRoute\0")) {
    checkRemoveRoute(command);
  } else {
    if (isUInt(beginWith) && (unsigned)strtol(beginWith, NULL, 10)) {
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