#ifndef CONTDYNARRAY_H
#define CONTDYNARRAY_H

// ContDynArray.h
//
// UE Algorithmen und Datenstrukturen - SS 2016 Universitaet Wien
// https://cewebs.cs.univie.ac.at/ADS/ss16/
//
// Beispielimplementierung mit Datenstruktur "unsortiertes Feld"
// Achtung: Da hier keine effiziente Suchdatenstruktur zum Einsatz kommt,
// ist das Laufzeitverhalten extrem schlecht.

#include <iostream>
#include "Container.h"
#include <limits.h> //For CHAR_BIT
#include <random> //for random number gen

class ContDynArrayEmptyException: public ContainerException {
public:
        virtual const char * what() const noexcept override {
                return "ContDynArray: empty";
        }
};

template <typename E, size_t N=16>
                                class ContDynArray:public Container<E> {
        size_t nmax, oldnmax, n, q;

        E * H1;
        E * H2;

        enum class Status:char { frei, belegt, wiederfrei };
        Status * s1;
        Status * s2;


        //std::random_device rd; // only used once to initialise (seed) engine
        std::default_random_engine rd;
        size_t a1 = random_nmbr(); //random numbers should be huge about 18-20 digits
        size_t a2 = random_nmbr();

        size_t random_nmbr(){
                std::mt19937 rng(rd()); // random-number engine used (Mersenne-Twister in this case)
                std::uniform_int_distribution<size_t> distribution(0,SIZE_MAX-1); // guaranteed unbiased

                return distribution(rd)|1;
        }

// function for calculation of hash
        size_t hash1(const E& e) const {
                return q ? ((a1*hashValue(e))>>((CHAR_BIT*sizeof(size_t))-q)) : 0;
        }
        size_t hash2(const E& e) const {
                return q ? ((a2*hashValue(e))>>((CHAR_BIT*sizeof(size_t))-q)) : 0;
        }

//rewrite with pot
        size_t pot(size_t size) {
                size_t h=0;
                size_t th=h;
                while(size>(h)) {
                        ++th;
                        h=(1<<th);
                }
                return (h);
        }

        size_t expt(size_t size) {
                size_t h=0;
                size_t th=h;
                while(size>(h)) {
                        ++th;
                        h=(1<<th);
                }
                return (th);
        }

        void add_(const E&, size_t t);

        void resize(size_t nmaxnew);
        void rehash(const E&);


        bool member1_(const E& e) const;
        bool member2_(const E& e) const;

        void sort(size_t l, size_t r, E* applyval) const;
public:
        ContDynArray() : nmax {(N<1) ? 2 : pot(N)}, n {0},q {expt(N)}, H1 {new E[this->nmax]()}, H2 {new E[this->nmax]()},s1 {new Status[this->nmax]()},s2 {new Status[this->nmax]()} {
        }
        ContDynArray(std::initializer_list<E> el) : ContDynArray() {
                for (auto e: el) add(e);
        }

        virtual ~ContDynArray() {
                delete[] H1;
                delete[] H2;
                delete[] s1;
                delete[] s2;
        }

        using Container<E>::add;
        virtual void add(const E e[], size_t len) override;

        using Container<E>::remove;
        virtual void remove(const E e[], size_t len) override;

        virtual bool member(const E& e) const override;
        virtual size_t size() const override {
                return n;
        }

        virtual E min() const override;
        virtual E max() const override;

        virtual std::ostream& print(std::ostream& o) const override;

        virtual size_t apply(std::function<void(const E &)> f, Order order = dontcare) const override;
        void swap( E&, E&) const;
};

template <typename E, size_t N>
void ContDynArray<E,N>::add_(const E &e,size_t t) {//Try with a loop now as recurions seem to increase stack size way beyond limit
        if (n  > (nmax*0.48))
                resize(size_t(pow(2,++q)));

        E element = e;
        while(t) {//as long as we do not have to rehash
                size_t pos1 = hash1(element);

                if (s1[pos1] != Status::belegt) {//wenn h1 and entsprechender stelle frei ist
                        H1[pos1] = element; //insert elemnt into  H1
                        s1[pos1] = Status::belegt;
                        ++n;
                        break;
                }
                else{//wenn h1 and entsprechender stelle belegt ist
                        E tmp = H1[pos1]; //store the element from H1 in a tmp variable
                        H1[pos1] = element; //insert elemnt into  H1
                        if(s2[hash2(tmp)] != Status::belegt) {//h2 and entsprechender stelle ist frei
                                H2[hash2(tmp)] = tmp; //move the conflicting element from tmp to H2
                                s2[hash2(tmp)] = Status::belegt;
                                ++n;
                                break;
                        }
                        else{ //both H1 and H2 have something at that their corresponding positions we now need to juggle this arround
                                element = H2[hash2(tmp)]; //store the element from H2 in a tmp variable
                                H2[hash2(tmp)] = tmp; //move the conflicting element from tmp to H2
                                --t;
                        }
                }
        }
        if(t==0) {
                //std::cout << "We need to rehash! \n";
                rehash(element);
        }
}

template <typename E, size_t N>
void ContDynArray<E,N>::add(const E e[], size_t len) {



        for (size_t i = 0; i < len; ++i) { //go through all values we where given
                if (!member(e[i])) { //only execute ifhe element is neither in H1 nor in H2
                        add_(e[i],(nmax/2+1)); //count=tabellesize/2+1;
                }
        }
}

template <typename E, size_t N>
void ContDynArray<E,N>::resize(size_t nmaxnew) {
        oldnmax = nmax;//das brauchen wir um über die alten H1 und H2 zu iterieren
        nmax = nmaxnew; //nmax als nächster 2er-potenz setzen
        //std::cout << "oldnamx: " << oldnmax << " nmaxnew: " << nmaxnew <<"\n";
        //save old stuff and allocate space for new stuff
        E * old_H1 = H1;
        E * old_H2 = H2;
        Status * old_s1 = s1;
        Status * old_s2 = s2;
        H1 = new E[nmax]();
        H2 = new E[nmax]();
        s1 = new Status[nmax]();
        s2 = new Status[nmax]();
        n=0;
        //end of save old stuff and allocate space for new stuff

        //write old elements into new hashtables
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s1[i] == Status::belegt) add(old_H1[i]);
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s2[i] == Status::belegt) add(old_H2[i]);

        //delete temp arrays
        delete[] old_H1;
        delete[] old_H2;
        delete[] old_s1;
        delete[] old_s2;
}

template <typename E, size_t N>
void ContDynArray<E,N>::rehash(const E &e) {
        a1 = random_nmbr(); //random numbers should be huge about 18-20 digits
        a2 = random_nmbr();
        resize(nmax);//call resize to rewrite all elements
        add(e);

}

template <typename E, size_t N>
void ContDynArray<E,N>::remove(const E e[], size_t len) {
        for (size_t i = 0; i < len; ++i) {
                if (member1_(e[i])) {
                        s1[hash1(e[i])] = Status::wiederfrei;
                        --n;
                }
                else if (member2_(e[i])) {
                        s2[hash2(e[i])] = Status::wiederfrei;
                        --n;
                }
        }
}

template <typename E, size_t N>
bool ContDynArray<E,N>::member1_(const E &e) const {
        if(s1[hash1(e)] == Status::belegt && H1[hash1(e)] == e )
                return true;
        else
                return false;
}

template <typename E, size_t N>
bool ContDynArray<E,N>::member2_(const E &e) const {
        if(s2[hash2(e)] == Status::belegt && H2[hash2(e)] == e )
                return true;
        else
                return false;
}

template <typename E, size_t N>
bool ContDynArray<E,N>::member(const E &e) const {
        if(member1_(e) || member2_(e))
                return true;
        else
                return false;
}

template <typename E, size_t N>
E ContDynArray<E,N>::min() const {
        if (this->empty()) throw ContDynArrayEmptyException();
        E * rc = nullptr;
        for (size_t i = 0; i < nmax; ++i) {
                if (s1[i] == Status::belegt && (!rc || *rc > H1[i]))
                        rc = &H1[i];
        }
        for (size_t i = 0; i < nmax; ++i) {
                if (s2[i] == Status::belegt && (!rc || *rc > H2[i] ))
                        rc = &H2[i];
        }
        return *rc;
}

template <typename E, size_t N>
E ContDynArray<E,N>::max() const {
        if (this->empty()) throw ContDynArrayEmptyException();
        E * rc = nullptr;
        for (size_t i = 0; i < nmax; ++i) {
                if (s1[i] == Status::belegt && (!rc || H1[i] > *rc))
                        rc = &H1[i];
        }
        for (size_t i = 0; i < nmax; ++i) {
                if (s2[i] == Status::belegt && (!rc || H2[i] > *rc))
                        rc = &H2[i];
        }
        return *rc;
}

template <typename E, size_t N>
std::ostream& ContDynArray<E,N>::print(std::ostream& o) const {
        o << "Cuckoo Hashing nmax=" << nmax << " n= " << n << "\n";
        o << "H1\n";
        for (size_t i = 0; i < nmax; ++i) {
                o << i << '[' << int(s1[i]) << "] " << H1[i] << '\n';
        }
        o << "H2\n";
        for (size_t i = 0; i < nmax; ++i) {
                o << i << '[' << int(s2[i]) << "] " << H2[i] << '\n';
        }
        return o;
}

template <typename E, size_t N>
size_t ContDynArray<E,N>::apply(std::function<void(const E &)> f, Order order) const {
        size_t rc = 0;
        if (order == dontcare) {
                try {
                        for (size_t i = 0; i < nmax; ++i) {
                                if (s1[i] == Status::belegt) {
                                        f(H1[i]);
                                        ++rc;
                                }
                        }
                        for (size_t i = 0; i < nmax; ++i) {
                                if (s2[i] == Status::belegt) {
                                        f(H2[i]);
                                        ++rc;
                                }
                        }
                } catch (...) {
                }
        } else {
                E * values = new E[n];
                size_t x = 0;
                for (size_t i = 0; i < nmax; ++i) {
                        if (s1[i] == Status::belegt) {
                                values[x++] = H1[i];
                        }

                }
                for (size_t i = 0; i < nmax; ++i) {
                        if (s2[i] == Status::belegt) {
                                values[x++] = H2[i];
                        }
                }
                sort(0,n-1, values);
                try {
                        if (order == descending) {
                                for (size_t i = n; i--; ) {
                                        f(values[i]);
                                        ++rc;
                                }
                        } else {
                                for (size_t i = 0; i < n; ++i) {
                                        f(values[i]);
                                        ++rc;
                                }
                        }
                } catch (...) {
                }
                delete[] values;
        }
        return rc;



}

template <typename E, size_t N>
void ContDynArray<E,N>::swap(E &x, E &y) const {
        E help = x;
        x = y;
        y = help;
}

template <typename E, size_t N>
void ContDynArray<E,N>::sort(size_t l, size_t r, E* applyval) const {  // Achtung, O(n*n)

        size_t i = l, j = r;
        E tmp;
    //    std::cout << "\n\n" << ((l + r) / 2) << "\n\n";
        E pivot = applyval[(l + r) / 2];

        //partitionieren
        while (i <= j) {
                while (pivot > applyval[i])//move left
                        i++;
                while (applyval[j] > pivot) {
                        if(j==0) {//stupid check so we dont go to size_t limit
                                break;
                        }
                        j--;
                }

                if (i <= j) {
                      //  std::cout << "\n\n i: " << i << " j: " << j << "\n\n";
                        std::swap(applyval[i], applyval[j]);
                        i++;
                        if(j==0) {//stupid check so we dont go to size_t limit
                                break;
                        }
                        j--;
                }
        }

        // rekursic aufrufen
        if (l < j)
                sort(l, j, applyval);
        if (i < r)
                sort(i, r, applyval);


}



#endif //CONTDYNARRAY_H
