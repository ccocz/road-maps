/** @file
 * Interfejs klasy przechowującej mapę dróg krajowych
 *
 * @author Łukasz Kamiński <kamis@mimuw.edu.pl>, Marcin Peczarski <marpe@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 20.03.2019
 */

#ifndef __MAP_H__
#define __MAP_H__

#include <stdbool.h>
#include <stdint.h>
#include "heap.h"

#define N 7213  /**<Lucky number for division of cities for hashing*/
#define R 1000  /**<Maximum possible route id plus one*/

typedef struct Road Road;          /**<Structure for list of roads*/
typedef struct City City;          /**<Structure for city*/
typedef struct HeapNode HeapNode;  /**<Structure for heap node*/
typedef struct Route Route;        /**<Structure for route*/
typedef struct Edges Edges;        /**<Structure for list of edges*/
typedef struct Routes Routes;      /**<Structure for list of routes*/

/**
 * @brief Structure for city
 */
struct City {
  char *name;           /**<Name of the city*/
  City *next;           /**<Next city in the list*/
  bool allowed;         /**<Allowance of the city*/
  HeapNode *heapNode;   /**<Address of the city in heap*/
  Edges *edges;         /**<Adjacent cities list*/
};
/**
 * @brief Structure for road between cities
 */
struct Road {
  City *from;           /**<Beginning of the road*/
  City *to;             /**<Ending of the road*/
  unsigned length;      /**<Length of the road*/
  int year;             /**<Length of the road*/
  Routes *routes;       /**<Routes from which road passes*/
  Road *next;           /**<Next road in the list*/
  Road *prev;           /**<Previous road in the list*/
};
/**
 * @brief Structure for list of routes
 */
struct Routes {
  unsigned routeId;     /**<id of the route*/
  Routes *next;         /**<Next route in the list*/
};
/**
 * @brief Structure for edges
 */
struct Edges{
  Road *road;           /**<Address of the road*/
  Edges *prev;          /**<Previous road in the list*/
  Edges *next;          /**<Next road in the list*/
};
/**
 * @brief Structure for route
 */
struct Route {
  City *start;          /**<Starting city of the route*/
  City *end;            /**<Ending city of the route*/
  uint64_t totalCost;   /**<Total length of the route*/
  int year;             /**<Oldest year of the route*/
  Edges *edges;         /**<List of edges in the route*/
};
/**
 * @brief Structure for whole of the map
 */
struct Map{
  City *cities[N];      /**<Array of cities on the map*/
  Route *routes[R];     /**<Array of routes on the map*/
  Road *roads;          /**<List of roads on the map*/
};

/**
 * Struktura przechowująca mapę dróg krajowych.
 */
typedef struct Map Map;

/** @brief Tworzy nową strukturę.
 * Tworzy nową, pustą strukturę niezawierającą żadnych miast, odcinków dróg ani
 * dróg krajowych.
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci.
 */
Map* newMap(void);

/** @brief Usuwa strukturę.
 * Usuwa strukturę wskazywaną przez @p map.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] map        – wskaźnik na usuwaną strukturę.
 */
void deleteMap(Map *map);

/** @brief Dodaje do mapy odcinek drogi między dwoma różnymi miastami.
 * Jeśli któreś z podanych miast nie istnieje, to dodaje go do mapy, a następnie
 * dodaje do mapy odcinek drogi między tymi miastami.
 * @param[in,out] map    – wskaźnik na strukturę przechowującą mapę dróg;
 * @param[in] city1      – wskaźnik na napis reprezentujący nazwę miasta;
 * @param[in] city2      – wskaźnik na napis reprezentujący nazwę miasta;
 * @param[in] length     – długość w km odcinka drogi;
 * @param[in] builtYear  – rok budowy odcinka drogi.
 * @return Wartość @p true, jeśli odcinek drogi został dodany.
 * Wartość @p false, jeśli wystąpił błąd: któryś z parametrów ma niepoprawną
 * wartość, obie podane nazwy miast są identyczne, odcinek drogi między tymi
 * miastami już istnieje lub nie udało się zaalokować pamięci.
 */
bool addRoad(Map *map, const char *city1, const char *city2,
             unsigned length, int builtYear);

/** @brief Modyfikuje rok ostatniego remontu odcinka drogi.
 * Dla odcinka drogi między dwoma miastami zmienia rok jego ostatniego remontu
 * lub ustawia ten rok, jeśli odcinek nie był jeszcze remontowany.
 * @param[in,out] map    – wskaźnik na strukturę przechowującą mapę dróg;
 * @param[in] city1      – wskaźnik na napis reprezentujący nazwę miasta;
 * @param[in] city2      – wskaźnik na napis reprezentujący nazwę miasta;
 * @param[in] repairYear – rok ostatniego remontu odcinka drogi.
 * @return Wartość @p true, jeśli modyfikacja się powiodła.
 * Wartość @p false, jeśli wystąpił błąd: któryś z parametrów ma niepoprawną
 * wartość, któreś z podanych miast nie istnieje, nie ma odcinka drogi między
 * podanymi miastami, podany rok jest wcześniejszy niż zapisany dla tego odcinka
 * drogi rok budowy lub ostatniego remontu.
 */
bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear);

/** @brief Łączy dwa różne miasta drogą krajową.
 * Tworzy drogę krajową pomiędzy dwoma miastami i nadaje jej podany numer.
 * Wśród istniejących odcinków dróg wyszukuje najkrótszą drogę. Jeśli jest
 * więcej niż jeden sposób takiego wyboru, to dla każdego wariantu wyznacza
 * wśród wybranych w nim odcinków dróg ten, który był najdawniej wybudowany lub
 * remontowany i wybiera wariant z odcinkiem, który jest najmłodszy.
 * @param[in,out] map    – wskaźnik na strukturę przechowującą mapę dróg;
 * @param[in] routeId    – numer drogi krajowej;
 * @param[in] city1      – wskaźnik na napis reprezentujący nazwę miasta;
 * @param[in] city2      – wskaźnik na napis reprezentujący nazwę miasta.
 * @return Wartość @p true, jeśli droga krajowa została utworzona.
 * Wartość @p false, jeśli wystąpił błąd: któryś z parametrów ma niepoprawną
 * wartość, istnieje już droga krajowa o podanym numerze, któreś z podanych
 * miast nie istnieje, obie podane nazwy miast są identyczne, nie można
 * jednoznacznie wyznaczyć drogi krajowej między podanymi miastami lub nie udało
 * się zaalokować pamięci.
 */
bool newRoute(Map *map, unsigned routeId,
              const char *city1, const char *city2);

/** @brief Wydłuża drogę krajową do podanego miasta.
 * Dodaje do drogi krajowej nowe odcinki dróg do podanego miasta w taki sposób,
 * aby nowy fragment drogi krajowej był najkrótszy. Jeśli jest więcej niż jeden
 * sposób takiego wydłużenia, to dla każdego wariantu wyznacza wśród dodawanych
 * odcinków dróg ten, który był najdawniej wybudowany lub remontowany i wybiera
 * wariant z odcinkiem, który jest najmłodszy.
 * @param[in,out] map    – wskaźnik na strukturę przechowującą mapę dróg;
 * @param[in] routeId    – numer drogi krajowej;
 * @param[in] city       – wskaźnik na napis reprezentujący nazwę miasta.
 * @return Wartość @p true, jeśli droga krajowa została wydłużona.
 * Wartość @p false, jeśli wystąpił błąd: któryś z parametrów ma niepoprawną
 * nazwę, nie istnieje droga krajowa o podanym numerze, nie ma miasta o podanej
 * nazwie, przez podane miasto już przechodzi droga krajowa o podanym numerze,
 * podana droga krajowa kończy się w podanym mieście, nie można jednoznacznie
 * wyznaczyć nowego fragmentu drogi krajowej lub nie udało się zaalokować
 * pamięci.
 */
bool extendRoute(Map *map, unsigned routeId, const char *city);

/** @brief Usuwa odcinek drogi między dwoma różnymi miastami.
 * Usuwa odcinek drogi między dwoma miastami. Jeśli usunięcie tego odcinka drogi
 * powoduje przerwanie ciągu jakiejś drogi krajowej, to uzupełnia ją
 * istniejącymi odcinkami dróg w taki sposób, aby była najkrótsza. Jeśli jest
 * więcej niż jeden sposób takiego uzupełnienia, to dla każdego wariantu
 * wyznacza wśród dodawanych odcinków drogi ten, który był najdawniej wybudowany
 * lub remontowany i wybiera wariant z odcinkiem, który jest najmłodszy.
 * @param[in,out] map    – wskaźnik na strukturę przechowującą mapę dróg;
 * @param[in] city1      – wskaźnik na napis reprezentujący nazwę miasta;
 * @param[in] city2      – wskaźnik na napis reprezentujący nazwę miasta.
 * @return Wartość @p true, jeśli odcinek drogi został usunięty.
 * Wartość @p false, jeśli z powodu błędu nie można usunąć tego odcinka drogi:
 * któryś z parametrów ma niepoprawną wartość, nie ma któregoś z podanych miast,
 * nie istnieje droga między podanymi miastami, nie da się jednoznacznie
 * uzupełnić przerwanego ciągu drogi krajowej lub nie udało się zaalokować
 * pamięci.
 */
bool removeRoad(Map *map, const char *city1, const char *city2);

/** @brief Udostępnia informacje o drodze krajowej.
 * Zwraca wskaźnik na napis, który zawiera informacje o drodze krajowej. Alokuje
 * pamięć na ten napis. Zwraca pusty napis, jeśli nie istnieje droga krajowa
 * o podanym numerze. Zaalokowaną pamięć trzeba zwolnić za pomocą funkcji free.
 * Informacje wypisywane są w formacie:
 * numer drogi krajowej;nazwa miasta;długość odcinka drogi;rok budowy lub
 * ostatniego remontu;nazwa miasta;długość odcinka drogi;rok budowy lub
 * ostatniego remontu;nazwa miasta;…;nazwa miasta.
 * Kolejność miast na liście jest taka, aby miasta @p city1 i @p city2, podane
 * w wywołaniu funkcji @ref newRoute, które utworzyło tę drogę krajową, zostały
 * wypisane w tej kolejności.
 * @param[in,out] map    – wskaźnik na strukturę przechowującą mapę dróg;
 * @param[in] routeId    – numer drogi krajowej.
 * @return Wskaźnik na napis lub NULL, gdy nie udało się zaalokować pamięci.
 */
char const* getRouteDescription(Map *map, unsigned routeId);

/**
 * @brief Usuwa z mapy dróg drogę krajową o podanym numerze, jeśli taka
 * istnieje, dając wynik true, a w przeciwnym przypadku, tzn. gdy podana
 * droga krajowa nie istnieje lub podany numer jest niepoprawny, niczego nie
 * zmienia w mapie dróg, dając wynik false. Nie usuwa odcinków dróg ani miast.
 * @param[in,out] map    – wskaźnik na strukturę przechowującą mapę dróg;
 * @param[in] routeId    – numer drogi krajowej.
 * @return Wartość @p true jeśli droga krajowa została usunięty, a w przeciwnym
 * przypadku tzn. gdy podana droga krajowa nie istnieje lub podany numer jest
 * niepoprawny, wartość @p false.
 */
bool removeRoute(Map *map, unsigned routeId);

#endif /* __MAP_H__ */
