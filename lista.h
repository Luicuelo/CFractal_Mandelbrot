#define null  0


typedef struct _Obj
{
  struct _Obj * sig;
  int x;
  int y;
  int height;
  int width;
  int valor;
  int v1;
  int v2;
  int v3;
  int v4;
  //de izquierda a derecha y de arriba a abajo.
} Obj;

typedef struct _lista
{
    int pos;
    struct _lista * sig;
    Obj * objetos;
} Lista;


// en la cabeza se guarda el valor m�s alto de la lista
// la lista esta ordenada desdendentemente


extern Lista memLista[300]; //300 nodos de lista alojados, maxiters debe ser menor que 300
extern Obj memObj[2000000]; //He tirado por lo alto :-), Si alojamos toda la memoria de tacada mas rapido que malloc y free.
extern Lista* cabL;

void freeO(void);
void freeL(void);
Obj * desapila(void);
void inserta (int pos,int valor,int x,int y, int width,int height,int v1,int v2,int v3,int v4);
