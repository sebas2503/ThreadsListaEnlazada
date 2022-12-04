//
//  main.cpp
//  ListaThreads
//
//  Created by Lucho on 11/26/22.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <ctime>
#include <queue>
using namespace std;
int get_random(int low, int high) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distribution(low, high);
    return distribution(gen);
}
struct ascendente
{
public:
    bool operator () (int x, int y)
    {
        return x <= y;
    }
};
class nodo
{
public:
    int valor;
    nodo* next;
    recursive_mutex candado;
    nodo(int v, nodo* n = nullptr)
    {
        valor = v;
        next = n;
    }
};

template <class T>
class LE
{
    nodo* head;
    nodo* tail;
    queue<nodo*> recolector_basura;
public:
    LE();
    void print();
    bool find(int, nodo*&);
    bool comond_find(int, nodo*&);
    bool add(int);
    bool del(int);
    bool contains(int);
    bool WeakSearch(nodo*&,nodo*&,int);
    bool StrongSearch(nodo*&,nodo*&,int);
    bool InsertThread(int);
    bool DeleteThread(int);
    bool PutOnGarbage(nodo*&);
    ~LE();
};
template<class T>
bool LE<T>::PutOnGarbage(nodo*& basura)
{
    recolector_basura.push(basura);
    return 1;
}
template<class T>
LE<T>::LE()
{
    head = new nodo(-2147483648);
    tail = new nodo(2147483647);
    head->next  = tail;
}
template <class T>
void LE<T>::print()
{
    cout << "Head->";
    for (nodo* p = head->next;p != tail; p = p->next)
        cout << p->valor << "->";
    cout << "NULL" << endl;
}
template <class T>
bool LE<T>::contains(int v)
{
    nodo* pos_ant;
    if (find(v, pos_ant))
        return true;
    return false;
}
template<class T>
bool LE<T>::WeakSearch(nodo*& padre,nodo*& hijo,int valor)
{
    padre = head;
    hijo = padre->next;
    //hijo != tail &&
    while (hijo->valor < valor)
    {
        padre = hijo;
        hijo = padre->next;
    }
    return 1;
}
template<class T>
bool LE<T>::StrongSearch(nodo*&padre,nodo*&hijo,int valor)
{
    WeakSearch(padre,hijo,valor);
    padre->candado.lock();
    hijo = padre->next;
    //hijo != tail && (hijo->valor < valor && padre->valor < valor)
    while (hijo->valor <valor)
    {
        padre->candado.unlock();
        padre = hijo;
        padre->candado.lock();
        hijo = padre->next;
    }
    return 1;
}
template<class T>
bool LE<T>::InsertThread(int valor)
{
    nodo*padre;
    nodo*hijo;
    nodo* aux;
    StrongSearch(padre,hijo,valor);
    if (hijo->valor == valor)
    {
        hijo->valor = valor;
    }
    else
    {
        aux = new nodo(valor);
        padre->next = aux;
        aux->next = hijo;
    }
    padre->candado.unlock();
    return 1;
}
template<class T>
bool LE<T>::DeleteThread(int valor)
{
    nodo* padre;
    nodo* hijo;
    StrongSearch(padre,hijo,valor);
    if(hijo->valor == valor)
    {
        hijo->candado.lock();
        padre->next = hijo->next;
        hijo->next = padre;
        hijo->candado.unlock();
        PutOnGarbage(hijo);
        //delete hijo;
    }
    padre->candado.unlock();
    return 1;
}
template <class T>
bool LE<T>::find(int v, nodo*& pos)
{
    T comp;
    bool encontrado = false;
    pos = head;
    for (nodo* p = head; p != tail && comp(p->valor, v); p = p->next)
    {
        if (p->valor == v)
        {
            encontrado = true;
            break;
        }
        pos = p;
    }
    return encontrado;
}
template <class T>
bool LE<T>::add(int v)
{

    nodo* pos_ant;
    if (!find(v, pos_ant))
    {
        pos_ant->next = new nodo(v,pos_ant->next);
    }
    return false;
}

template <class T>
bool LE<T>::del(int v)
{
    nodo* pos_ant, * pos_del;
    if (find(v, pos_ant))
    {
        pos_del = pos_ant->next;
        pos_ant->next = pos_del->next;
        delete pos_del;
    }
    return false;
}

template <class T>
LE<T>::~LE()
{
    while (head->next != tail)
        del(head->next->valor);
    delete tail;
    delete head;
}

//Functores y fin de la implementacion de la lista.
template<class T>
struct Add
{
    LE<T>* ptr;
    int min;
    int max;
    Add(int min, int max, LE<T>* lista)
    {
        this->min = min;
        this->max = max;
        ptr = lista;
    }
    void operator ()(int NumOpe)
    {
        int x = 0;
        for (int i = 0; i < NumOpe; i++)
        {
            x = get_random(min, max);
            ptr->InsertThread(x);
        }
    }
};
template<class T>
struct Delete
{
    LE<T>* ptr;
    int min;
    int max;
    Delete(int min, int max, LE<T>* lista)
    {
        this->min = min;
        this->max = max;
        ptr = lista;
    }
    void operator ()(int NumOpe)
    {
        int x = 0;
        for (int i = 0; i < NumOpe; i++)
        {
            x = get_random(min, max);
            ptr->DeleteThread(x);
        }
    }
};

int main(int argc, const char* argv[])
{
    LE<ascendente> lista;
    thread* mythread[5];
    unsigned t0, t1;
    t0=clock();
    mythread[0] = new thread(Add<ascendente>(1,100, &lista),10);
    mythread[1] = new thread(Add<ascendente>(1,100,&lista),10);
    mythread[2] = new thread(Add<ascendente>(1,100, &lista),10);
    mythread[3] = new thread(Delete<ascendente>(1,100,&lista),40);
    mythread[4] = new thread(Delete<ascendente>(1,100,&lista),40);
    mythread[0]->join();
    mythread[1]->join();
    mythread[2]->join();
    mythread[3]->join();
    mythread[4]->join();
    lista.print();
    t1 = clock();
    double time = (double(t1-t0)/CLOCKS_PER_SEC);
    cout << "Execution Time: " << time << endl;
    return 0;
}
