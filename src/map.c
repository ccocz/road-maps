#include "map.h"
#include "heap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void freeRoutes(Routes *routes) {
  Routes *help;
  while (routes) {
    help = routes;
    routes = routes->next;
    free(help);
  }
}

void freeEdges(Edges *this) {
  Edges *edges = this, *helpEdges;
  while (edges) {
    helpEdges = edges;
    edges = edges->next;
    free(helpEdges);
  }
}

void freeRoute(Route *route) {
  freeEdges(route->edges);
  free(route);
}

void freeMap(Map *map) {
  City *aux, *help;
  for (int i = 0; i < N; i++) {
    aux = map->cities[i];
    while (aux) {
      help = aux;
      freeEdges(aux->edges);
      free(aux->name);
      aux = aux->next;
      free(help);
    }
  }
  Route *route;
  for (int i = 0; i < R; i++) {
    route = map->routes[i];
    if (route) {
      freeRoute(route);
    }
  }
  Road *roads = map->roads, *helpRoads;
  while (roads) {
    helpRoads = roads;
    freeRoutes(roads->routes);
    roads = roads->next;
    free(helpRoads);
  }
  free(map);
}

void deleteMap(Map *map) {
  freeMap(map);
}

Map *newMap(void) {
  Map *aux = malloc(sizeof(Map));
  if (!aux) {
    return aux;
  } else {
    for (int i = 0; i < N; i++) {
      aux->cities[i] = NULL;
    }
    for (int i = 0; i < R; i++) {
      aux->routes[i] = NULL;
    }
    aux->roads = NULL;
    return aux;
  }
}

bool badName(const char *city) {
  size_t n = strlen(city);
  for (size_t i = 0; i < n; i++) {
    if ((city[i] >= 0 && city[i] <= 31) || city[i] == ';') {
      return true;
    }
  }
  return n == 0;
}

int MOD(int x, int mod) {
  while (x < 0) {
    x += mod;
  }
  return x;
}

int hashIt(const char *s) {
  size_t n = strlen(s);
  int mod = N;
  int base = 1453;
  int hash = 0;
  for (size_t i = 0; i < n; i++) {
    hash = (hash + ((1LL * MOD((int)s[i], mod) * base) % mod)) % mod;
    base = (base * 1453) % mod;
  }
  return hash;
}

City *cityExists(Map *map, const char *city) {
  int hash = hashIt(city);
  City *aux = map->cities[hash];
  while (aux) {
    if (!strcmp(aux->name, city)) {
      return aux;
    }
    aux = aux->next;
  }
  return NULL;
}

char *makeCopy(const char *city) {
  size_t n = strlen(city);
  char *copy = malloc(n + 1);
  for (size_t i = 0; i < n; i++) {
    copy[i] = city[i];
  }
  copy[n] = '\0';
  return copy;
}

City *addCity(Map *map, const char *city) {
  int hash = hashIt(city);
  City *where = map->cities[hash];
  City *aux = malloc(sizeof(City));
  aux->allowed = true;
  aux->name = makeCopy(city);
  aux->edges = NULL;
  aux->heapNode = NULL;
  aux->next = where;
  map->cities[hash] = aux;
  return aux;
}

Road *isConnected(City *city1, City *city2) {
  Edges *edges = city1->edges;
  while (edges) {
    if (edges->road->from == city2 || edges->road->to == city2) {
      return edges->road;
    }
    edges = edges->next;
  }
  return NULL;
}

void addEdge(City *city, Road *road) {
  Edges *help = malloc(sizeof(Edges));
  help->road = road;
  help->next = city->edges;
  if (city->edges) {
    city->edges->prev = help;
  }
  help->prev = NULL;
  city->edges = help;
}

void connectCities(Map *map, City *city1, City *city2,
                   unsigned length, int builtYear) {
  Road *aux = malloc(sizeof(Road));
  aux->from = city1;
  aux->to = city2;
  aux->length = length;
  aux->year = builtYear;
  aux->routes = NULL;
  aux->prev = NULL;
  aux->next = map->roads;
  if (map->roads) {
    map->roads->prev = aux;
  }
  map->roads = aux;
  addEdge(city1, aux);
  addEdge(city2, aux);
}

bool addRoad(Map *map, const char *city1, const char *city2,
             unsigned length, int builtYear) {
  if (badName(city1) || badName(city2) || !builtYear || !length) {
    return false;
  }
  if (!strcmp(city1, city2)) {
    return false;
  }
  City *first = cityExists(map, city1), *second = cityExists(map, city2);
  if (!first) {
    first = addCity(map, city1);
  }
  if (!second) {
    second = addCity(map, city2);
  }
  Road *road = isConnected(first, second);
  if (road) {
    return false;
  }
  connectCities(map, first, second, length, builtYear);
  return true;
}

bool repairRoad(Map *map, const char *city1,
                const char *city2, int repairYear){
  if (badName(city1) || badName(city2) || !repairYear) {
    return false;
  }
  if (!strcmp(city1, city2)) {
    return false;
  }
  City *first = cityExists(map, city1);
  City *second = cityExists(map, city2);
  if (!first || !second) {
    return false;
  }
  Road *go = isConnected(first, second);
  if (!go) {
    return false;
  }
  if (go->year > repairYear) {
    return false;
  }
  go->year = repairYear;
  return true;
}

bool maxi(int x, int y) {
  return x >= y;
}

int getMini(int x, int y) {
  return x < y ? x : y;
}

City *toCity(Road *road, City *from) {
  if (road->to == from) {
    return road->from;
  } else {
    return road->to;
  }
}

void dijkstra(Heap *Q, Road *banned) {
  HeapNode *best;
  Edges *adj;
  Road *adjRoad;
  City *adjCity;
  while (Q->root) {
    best = minHeap(Q);
    adj = best->city->edges;
    while (adj) {
      adjRoad = adj->road;
      adjCity = toCity(adjRoad, best->city);
      if (adjRoad != banned && adjCity->allowed &&
          !adjCity->heapNode->visited ) {
        if (best->distance + adjRoad->length < adjCity->heapNode->distance) {
          decreaseValue(adjCity->heapNode, best->distance + adjRoad->length,
                        getMini(best->year, adjRoad->year));
          adjCity->heapNode->from = adjRoad;
        } else if (best->distance + adjRoad->length == adjCity->heapNode->distance) {
          if (maxi(getMini(best->year, adjRoad->year), adjCity->heapNode->year)) {
            decreaseValue(adjCity->heapNode, best->distance + adjRoad->length,
                          getMini(best->year, adjRoad->year));
            adjCity->heapNode->from = adjRoad;
          }
        }
      }
      adj = adj->next;
    }
  }
}

bool checkUnique(Route *route, Road *banned) {
  City *start = route->start;
  City *helpCity;
  City *nextCity;
  City *prevCity = NULL;
  Edges *edges = route->edges;
  Edges *helpEdges;
  while (start) {
    if (edges) {
      nextCity = toCity(edges->road, start);
    } else {
      nextCity = NULL;
    }
    helpEdges = start->edges;
    while (helpEdges) {
      helpCity = toCity(helpEdges->road, start);
      if (helpEdges->road != banned && helpCity->allowed &&
          helpCity->heapNode && helpCity != nextCity && helpCity != prevCity) {
        if (helpCity->heapNode->distance + helpEdges->road->length == start->heapNode->distance &&
            getMini(helpCity->heapNode->year, helpEdges->road->year) == start->heapNode->year) {
          return false;
        }
      }
      helpEdges = helpEdges->next;
    }
    if (edges) {
      prevCity = start;
      start = toCity(edges->road, start);
      edges = edges->next;
    } else {
      start = NULL;
    }
  }
  return true;
}

Route *makeRoute(City *source, City *destination) {
  Edges *edgesHelp;
  Route *ret = malloc(sizeof(Route));
  ret->start = source;
  ret->end = destination;
  ret->edges = NULL;
  ret->totalCost = destination->heapNode->distance;
  ret->year = destination->heapNode->year;
  City *traverse = destination;
  while (traverse) {
    if (traverse->heapNode->from) {
      edgesHelp = malloc(sizeof(Edges));
      edgesHelp->road = traverse->heapNode->from;
      edgesHelp->next = ret->edges;
      if (ret->edges) {
        ret->edges->prev = edgesHelp;
      }
      ret->edges = edgesHelp;
      traverse = toCity(traverse->heapNode->from, traverse);
    } else {
      traverse = NULL;
    }
  }
  ret->edges->prev = NULL;
  return ret;
}

void addHeap(Heap *Q, City *source, Road *banned) {
  Edges *edges;
  City *adjCity;
  edges = source->edges;
  while (edges) {
    adjCity = toCity(edges->road, source);
    if (edges->road != banned && !adjCity->heapNode && adjCity->allowed) {
      insertHeap(Q, adjCity);
      addHeap(Q, adjCity, banned);
    }
    edges = edges->next;
  }
}

void freeNodes(Heap *Q, City *source, Road *banned) {
  Edges *edges;
  City *adjCity;
  edges = source->edges;
  free(source->heapNode);
  source->heapNode = NULL;
  while (edges) {
    adjCity = toCity(edges->road, source);
    if (edges->road != banned && adjCity->heapNode) {
      freeNodes(Q, adjCity, banned);
    }
    edges = edges->next;
  }
}

Route *startDijkstra(City *source, City *destination, Road *banned) {
  Heap *Q = newHeap(source);
  addHeap(Q, source, banned);
  bool ok = true;
  Route *ret = NULL;
  if (destination->heapNode) {
    dijkstra(Q, banned);
  }
  if (!destination->heapNode) {
    ok = false;
  }
  if (ok) {
    ret = makeRoute(source, destination);
  }
  if (ok && !checkUnique(ret, banned)) {
    freeRoute(ret);
    ret = NULL;
  }
  freeNodes(Q, source, banned);
  freeHeap(Q);
  return ret;
}

bool existId(Road *road, unsigned id) {
  Routes *routes = road->routes;
  while (routes) {
    if (routes->routeId == id) {
      return true;
    }
    routes = routes->next;
  }
  return false;
}

void giveId(Route *route, unsigned routeId) {
  Edges *edges = route->edges;
  Routes *aux;
  while (edges) {
    if (!existId(edges->road, routeId)) {
      aux = malloc(sizeof(Routes));
      aux->routeId = routeId;
      aux->next = edges->road->routes;
      edges->road->routes = aux;
    }
    edges = edges->next;
  }
}

bool newRoute(Map *map, unsigned routeId,
              const char *city1, const char *city2) {
  if (routeId > 999 || routeId <= 0 || badName(city1) || badName(city2)) {
    return false;
  }
  if (map->routes[routeId]) {
    return false;
  }
  if (!strcmp(city1, city2)) {
    return false;
  }
  City *first = cityExists(map, city1);
  City *second = cityExists(map, city2);
  if (!first || !second) {
    return false;
  }
  Route *ans = startDijkstra(first, second, NULL);
  if (!ans) {
    return false;
  } else {
    map->routes[routeId] = ans;
    giveId(ans, routeId);
    return true;
  }
}

void switchAllowed(Route *route, bool allowed) {
  City *start = route->start;
  Edges *edges = route->edges;
  while (start) {
    start->allowed = allowed;
    if (edges) {
      start = toCity(edges->road, start);
      edges = edges->next;
    } else {
      start = NULL;
    }
  }
}

Route *mergeRoutes(Route *a, Route *b, bool start) {
  if (start) {
    Edges *use = b->edges;
    Edges *help;
    while (use) {
      help = use->next;
      a->edges->prev = use;
      use->next = a->edges;
      use->prev = help;
      a->edges = use;
      use = help;
    }
    a->start = b->end;
  } else {
    Edges *find = a->edges;
    while (find->next) {
      find = find->next;
    }
    b->edges->prev = find;
    find->next = b->edges;
    a->end = b->end;
  }
  free(b);
  return a;
}

bool extendRoute(Map *map, unsigned routeId, const char *city) {
  if (routeId > 999 || badName(city)) {
    return false;
  }
  if (!map->routes[routeId]) {
    return false;
  }
  City *first = cityExists(map, city);
  if (!first) {
    return false;
  }
  Route *my = map->routes[routeId];
  switchAllowed(map->routes[routeId], false);
  if (!first->allowed) {
    switchAllowed(my, true);
    return false;
  }
  my->start->allowed = true;
  Route *fromHead = startDijkstra(my->start, first, NULL);
  my->start->allowed = false;
  my->end->allowed = true;
  Route *fromTail = startDijkstra(my->end, first, NULL);
  switchAllowed(my, true);
  if (!fromHead && !fromTail) {
    return false;
  }
  else if (!fromTail) {
    map->routes[routeId] = mergeRoutes(my, fromHead, true);
  }
  else if (!fromHead){
    map->routes[routeId] = mergeRoutes(my, fromTail, false);
  }
  else {
    if (fromHead->totalCost < fromTail->totalCost) {
      map->routes[routeId] = mergeRoutes(my, fromHead, true);
      freeRoute(fromTail);
    } else if (fromHead->totalCost > fromTail->totalCost) {
      map->routes[routeId] = mergeRoutes(my, fromTail, false);
      freeRoute(fromHead);
    } else {
      if (fromHead->totalCost == fromTail->totalCost && maxi(fromHead->year,
          fromTail->year)) {
        map->routes[routeId] = mergeRoutes(my, fromHead, true);
        freeRoute(fromTail);
      } else if (fromHead->totalCost == fromTail->totalCost
                 && maxi(fromTail->year, fromHead->year)) {
        map->routes[routeId] = mergeRoutes(my, fromTail, false);
        freeRoute(fromHead);
      } else {
        freeRoute(fromHead);
        freeRoute(fromTail);
        return false;
      }
    }
  }
  giveId(map->routes[routeId], routeId);
  return true;
}

void deleteEdge(City *city, Road *road) {
  Edges *edges = city->edges;
  bool found = false;
  while (!found && edges) {
    if (edges->road == road) {
      if (edges->prev) {
        edges->prev->next = edges->next;
        if (edges->next) {
          edges->next->prev = edges->prev;
        }
      }
      else {
        city->edges = edges->next;
        if (city->edges) {
          city->edges->prev = NULL;
        }
      }
      free(edges);
      found = true;
    }
    if (!found) {
      edges = edges->next;
    }
  }
}

void changeRoute(Route *route, Route *with, Road *road, City *from) {
  bool found = false;
  Edges *edges = route->edges;
  Edges *use, *help, *last;
  Edges *lastWith = with->edges;
  City *start = route->start;
  while (lastWith->next) {
    lastWith = lastWith->next;
  }
  while (!found && edges) {
    if (edges->road == road) {
      if (start == from) {
        if (edges->prev) {
          edges->prev->next = with->edges;
          with->edges->prev = edges->prev;
        } else {
          route->edges = with->edges;
          with->edges->prev = NULL;
        }
        lastWith->next = edges->next;
        if (edges->next) {
          edges->next->prev = lastWith;
        }
      } else {
        use = with->edges;
        last = edges->next;
        while (use) {
          help = use->next;
          use->next = last;
          use->prev = help;
          last = use;
          use = help;
        }
        if (edges->prev) {
          edges->prev->next = lastWith;
          lastWith->prev = edges->prev;
        } else {
          route->edges = lastWith;
          lastWith->prev = NULL;
        }
        if (edges->next) {
          edges->next->prev = with->edges;
        }
      }
      free(edges);
      found = true;
    }
    if (!found) {
      start = toCity(edges->road, start);
      edges = edges->next;
    }
  }
}

void deleteRoad(Map *map, Road *road) {
  if (road->prev) {
    road->prev->next = road->next;
    if (road->next) {
      road->next->prev = road->prev;
    }
  } else {
    map->roads = road->next;
    if (map->roads) {
      map->roads->prev = NULL;
    }
  }
  if (road->routes) {
    freeRoutes(road->routes);
  }
  free(road);
}

bool removeRoad(Map *map, const char *city1, const char *city2) {
  if (badName(city1) || badName(city2)) {
    return false;
  }
  City *first = cityExists(map, city1);
  City *second = cityExists(map, city2);
  if ((!first || !second) || (first == second)) {
    return false;
  }
  Road *connects = isConnected(first, second);
  if (!connects) {
    return false;
  }
  if (!connects->routes) {
    deleteEdge(first, connects);
    deleteEdge(second, connects);
    deleteRoad(map, connects);
    return true;
  }
  Routes *use;
  Route *new = NULL;
  use = connects->routes;
  while (use) {
    switchAllowed(map->routes[use->routeId], false);
    first->allowed = second->allowed = true;
    new = startDijkstra(first, second, connects);
    switchAllowed(map->routes[use->routeId], true);
    if (!new) {
      return false;
    }
    freeRoute(new);
    use = use->next;
  }
  use = connects->routes;
  while (use) {
    switchAllowed(map->routes[use->routeId], false);
    first->allowed = second->allowed = true;
    new = startDijkstra(first, second, connects);
    changeRoute(map->routes[use->routeId], new, connects, first);
    switchAllowed(map->routes[use->routeId], true);
    giveId(map->routes[use->routeId], use->routeId);
    free(new);
    use = use->next;
  }
  deleteEdge(first, connects);
  deleteEdge(second, connects);
  deleteRoad(map, connects);
  return true;
}

size_t intLen(int x) {
  if (x < 0) {
    x *= -1;
    size_t ret = 0;
    while (x) {
      ret++;
      x /= 10;
    }
    ret++;
    return ret;
  } else {
    size_t ret = 0;
    while (x) {
      ret++;
      x /= 10;
    }
    return ret;
  }
}

size_t findSize(Route *route, unsigned routeId) {
  size_t ret = intLen(routeId) + 1;
  ret += strlen(route->start->name) + 1;
  City *start = route->start;
  Edges *edges = route->edges;
  while (start) {
    if (edges) {
      start = toCity(edges->road, start);
      ret += strlen(start->name) + 1;
      ret += intLen(edges->road->year) + 1;
      ret += intLen(edges->road->length) + 1;
      edges = edges->next;
    } else {
      start = NULL;
    }
  }
  return ret;
}

const char *getRouteDescription(Map *map, unsigned routeId) {
  if (routeId > 999 || !map->routes[routeId]) {
    char *x = malloc(1);
    x[0] = '\0';
    return x;
  }
  Route *route = map->routes[routeId];
  char *ret = calloc(findSize(route, routeId), sizeof(char));
  size_t total_size = 0;
  sprintf(ret + total_size, "%u%c", routeId, ';');
  total_size += intLen(routeId) + 1;
  City *start = route->start;
  Edges *edges = route->edges;
  sprintf(ret + total_size, "%s%c", route->start->name, ';');
  total_size += strlen(route->start->name) + 1;
  while (start) {
    if (edges) {
      start = toCity(edges->road, start);
      sprintf(ret + total_size, "%u%c", edges->road->length, ';');
      total_size += intLen(edges->road->length) + 1;
      sprintf(ret + total_size, "%d%c", edges->road->year, ';');
      total_size += intLen(edges->road->year) + 1;
      edges = edges->next;
      if (edges) {
        sprintf(ret + total_size, "%s%c", start->name, ';');
        total_size += strlen(start->name) + 1;
      } else {
        sprintf(ret + total_size, "%s", start->name);
        total_size += strlen(start->name);
      }
    } else {
      start = NULL;
    }
  }
  return ret;
}

void removeFromRoad(Road *road, unsigned routeId) {
  Routes *routes = road->routes;
  Routes *prev = NULL;
  bool found = false;
  while (!found && routes) {
    if (routes->routeId == routeId) {
      found = true;
      if (prev) {
        prev->next = routes->next;
      } else {
        road->routes = routes->next;
      }
      free(routes);
    }
    prev = routes;
    routes = routes->next;
  }
}

bool removeRoute(Map *map, unsigned routeId) {
  if (routeId > 999 || !routeId || !map->routes[routeId]) {
    return false;
  }
  Edges *edges = map->routes[routeId]->edges;
  while (edges) {
    removeFromRoad(edges->road, routeId);
    edges = edges->next;
  }
  freeRoute(map->routes[routeId]);
  map->routes[routeId] = NULL;
  return true;
}