// Macierz sąsiedztwa
// Data: 14.07.2013
// (C)2013 mgr Jerzy Wałaszek
//---------------------------

#include <iostream>
#include <iomanip>

using namespace std;

int main( )
{
  int n, m, i, j, v1, v2;
  char ** A;

  cin >> n >> m;         // Czytamy liczbę wierzchołków i krawędzi

  A = new char * [ n ];  // Tworzymy tablicę wskaźników

  for( i = 0; i < n; i++ )
    A [ i ] = new char [ n ]; // Tworzymy wiersze

  // Macierz wypełniamy zerami

  for( i = 0; i < n; i++ )
    for( j = 0; j < n; j++ ) A [ i ][ j ] = 0;

  // Odczytujemy kolejne definicje krawędzi

  for( i = 0; i < m; i++ )
  {
    cin >> v1 >> v2;    // Wierzchołek startowy i końcowy krawędzi
    A [ v1 ][ v2 ] = 1; // Krawędź v1->v2 obecna
  }

  cout << endl;

  // Wypisujemy zawartość macierzy sąsiedztwa

  cout << "   ";
  for( i = 0; i < n; i++ ) cout << setw ( 3 ) << i;
  cout << endl << endl;
  for( i = 0; i < n; i++ )
  {
    cout << setw ( 3 ) << i;
    for( j = 0; j < n; j++ ) cout << setw ( 3 ) << ( int ) A [ i ][ j ];
    cout << endl;
  }

  // Usuwamy macierz

  for( i = 0; i < n; i++ ) delete [ ] A [ i ];
  delete [ ] A;

  cout << endl;

  return 0;
}