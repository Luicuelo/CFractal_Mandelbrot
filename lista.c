#include "lista.h"
#include "constantes.h"
#ifndef __cplusplus
    #include <stdbool.h>
#endif

Lista memLista[300];
Obj memObj[2000000];
Lista* cabL=null;
Lista* lastL;
bool listaVacia=true;
int pO=-1;


void freeL(void){
    cabL=null;
    memLista[0].sig=null;
    memLista[0].objetos=null;
    listaVacia=true;
    lastL=cabL;
}

void freeO(void)
{
    pO=-1;
    //memset (memLista,0,sizeof(Lista)*301);
    freeL();
}


Lista* allocL(int pos){
    Lista * lt;
    lt=&memLista[pos];
    lt->pos=pos;
    lt->sig=null;
    return(lt);
}


Lista* insertaL(int pos){

    Lista* lt=cabL;
    Lista* last=null;
    Lista* l;

    if (listaVacia) {
        l=allocL(pos);
        cabL=l;
        listaVacia=false;
        return (l);
    }

    while ((lt!=null)&&((lt->pos)>pos)){
        last=lt;
        lt=lt->sig;
    }

    if ((lt!=null) && (lt->pos)==pos) {
        return (lt);
    }
    else{

        l=allocL(pos); //modificado para rehusar los elementos de la lista
        l->sig=lt;
        if(last==null) cabL=l;//el valor a insertar es el mas alto
        else last->sig=l;
        return(l);
    }
}


Obj* apila(void)
{
    Obj * ot;
    pO++;
    memObj[pO].sig=null;
    ot=&(memObj[pO]);
    return(ot);
}

 Obj * desapila(void){

    Obj * o=null;
    if (lastL!=null && lastL->objetos!=null) {
        o =lastL->objetos;
        (lastL->objetos)=((lastL->objetos)->sig);
        return(o);
    }
    else
    {
        lastL=cabL; //se agoto el ultimo nivel que estabamos usando y vamos a la cabecera
        if (lastL!=null && lastL->objetos==null){ //si no hay nodos en la cabecera avanzamos
            cabL->pos=-1;
            cabL=cabL->sig;
            lastL=cabL;
            return (desapila());
        }
        else
            if (cabL==null) // terminamos con todos los objetos guardados
                return(null);
            else
                return (desapila()); //quedan objetos en la cabecera

    }

}

void inserta (int pos,int valor,int x,int y, int width,int height,int v1,int v2,int v3,int v4){
    Obj* objeto;
    Lista * l;

    //int lim=wid/200;
    int lim=8;

    objeto=apila();
    objeto->x=x;
    objeto->y=y;
    objeto->width=width;
    objeto->height=height;
    objeto->v1=v1;
    objeto->v2=v2;
    objeto->v3=v3;
    objeto->v4=v4;
    objeto->valor=valor;

    if (pos>0) {
        l=insertaL (pos);
        objeto->sig=l->objetos;
        l->objetos=objeto;
    }

    // Los cuadrados de distancia 0 los metemos en otra cola
    // Los dibujaremos aparte
    // es lo que mas tarda y la mayoria no seran significativos
    else{

        if (width>lim && height>lim){ // si el cuadrado es peque�o suponemos que esta bien
            l=&memLista[0];
            objeto->sig=l->objetos;
            l->objetos=objeto;
        }
    }


}













