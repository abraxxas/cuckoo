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
 #include <math.h>       /* pow */
 #include <stdlib.h> // For random(), RAND_MAX
 #include <limits.h> //For CHAR_BIT
 #include <random> //for random number gen
 #include <algorithm> //find in vector




class ContDynArrayEmptyException: public ContainerException {
public:
        virtual const char * what() const noexcept override {
                return "ContDynArray: empty";
        }
};

template <typename E, size_t N=7>
                                class ContDynArray: public Container<E> {
        size_t nmax;
        size_t oldnmax = nmax;//das brauchen wir um über die alten H1 und H2 zu iterieren
        size_t n;

        const int tMax = 10000; // max number of random walk trials
        int t=0; //number of current walks
        size_t q;//exponent der aktuellen tabellengröße als 2er potenz
        E * H1;
        E * H2;
        E * H3;
        E * H4;

        enum class Status: char { frei, belegt, wiederfrei };
        Status * s1;
        Status * s2;
        Status * s3;
        Status * s4;


        std::random_device rd; // only used once to initialise (seed) engine


        size_t a1 = random_nmbr(); //random numbers should be huge about 18-20 digits
        size_t a2 = random_nmbr();
        size_t a3 = random_nmbr(); //random numbers should be huge about 18-20 digits
        size_t a4 = random_nmbr();

        size_t random_nmbr(){
                std::mt19937 rng(rd()); // random-number engine used (Mersenne-Twister in this case)
                std::uniform_int_distribution<size_t> uni(0,SIZE_MAX); // guaranteed unbiased

                return uni(rng);
        }

        // function for calculation of hash
        size_t hash1(const E& e) const {
                return q ? ((a1*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q)) : 0;
        }
        size_t hash2(const E& e) const {
                return q ? ((a2*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q)) : 0;
        }
        size_t hash3(const E& e) const {
                return q ? ((a3*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q)) : 0;
        }
        size_t hash4(const E& e) const {
                return q ? ((a4*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q)) : 0;
        }

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

        void add_(const E&);

        void resize(size_t nmaxnew);
        void rehash();


        bool member1_(const E& e) const;
        bool member2_(const E& e) const;
        bool member3_(const E& e) const;
        bool member4_(const E& e) const;

        void sort() const;
public:
        ContDynArray() : nmax {(N<1) ? 2 : pot(N)}, n {0},q {expt(N)}, H1 {new E[this->nmax]()}, H2 {new E[this->nmax]()},H3 {new E[this->nmax]()}, H4 {new E[this->nmax]()}, s1 {new Status[this->nmax]()},s2 {new Status[this->nmax]()}, s3 {new Status[this->nmax]()},s4 {new Status[this->nmax]()} {
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
};

template <typename E, size_t N>
void ContDynArray<E,N>::add_(const E &e) {
        size_t pos1 = hash1(e);

        if (s1[pos1] != Status::belegt) {//wenn h1 and entsprechender stelle frei ist
                H1[pos1] = e; //insert elemnt into  H1
                s1[pos1] = Status::belegt;
        }else{//wenn h1 and entsprechender stelle belegt ist
                E tmp = H1[pos1]; //store the element from H1 in a tmp variable
                H1[pos1] = e; //insert elemnt into  H1
                if(s2[hash2(tmp)] != Status::belegt) {//h2 and entsprechender stelle ist frei
                        H2[hash2(tmp)] = tmp; //move the conflicting element from tmp to H2
                        s2[hash2(tmp)] = Status::belegt;
                }
                else{ //both H1 and H2 have something at that their corresponding positions we now need to juggle this arround
                        E tmp2 = H2[hash2(tmp)]; //store the element from H2 in a tmp variable
                        H2[hash2(tmp)] = tmp; //move the conflicting element from tmp to H2
                        if(s3[hash3(tmp2)] != Status::belegt) {//h3 and entsprechender stelle ist frei
                                H3[hash3(tmp2)] = tmp2; //move the conflicting element from tmp to H2
                                s3[hash3(tmp2)] = Status::belegt;
                        }else{//H3 auch belegt
                                E tmp3 = H3[hash3(tmp2)]; //store conflicting element in tmp variable
                                H3[hash3(tmp2)] = tmp2; //move the conflicting element from tmp to H2
                                if(s4[hash4(tmp3)] != Status::belegt) {//H4 ist frei
                                        H4[hash4(tmp3)] = tmp3;
                                        s4[hash4(tmp3)] = Status::belegt;
                                }else{//H4 auch belegt
                                        E tmp4 = H4[hash4(tmp3)];
                                        H4[hash4(tmp3)] = tmp3;

                                        if(t > tMax) {//do we need to rehash?
                                                //std::cout << "We need to rehash!\n" << "t: " << t << "\n";
                                                t = 0;//reset t since we have rehashed
                                                rehash();
                                        }
                                        t++;
                                        //std::cout << "t++: " << t << "\n";
                                        add_(tmp4);//call add again to store the element from the tmp2 variable
                                }
                        }


                }
        }
}

template <typename E, size_t N>
void ContDynArray<E,N>::add(const E e[], size_t len) {
        if (n + len > nmax) {
                resize(size_t(pow(2,++q)));
        }


        for (size_t i = 0; i < len; ++i) { //go through all values we where given
                if (!member(e[i])) { //only execute ifhe element is neither in H1 nor in H2
                        t = 0;//set current walks to 0
                        add_(e[i]);
                        ++n;
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
        E * old_H3 = H3;
        E * old_H4 = H4;
        Status * old_s1 = s1;
        Status * old_s2 = s2;
        Status * old_s3 = s3;
        Status * old_s4 = s4;
        H1 = new E[nmax]();
        H2 = new E[nmax]();
        H3 = new E[nmax]();
        H4 = new E[nmax]();
        s1 = new Status[nmax]();
        s2 = new Status[nmax]();
        s3 = new Status[nmax]();
        s4 = new Status[nmax]();
        //end of save old stuff and allocate space for new stuff

        //write old elements into new hashtables
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s1[i] == Status::belegt) add_(old_H1[i]);
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s2[i] == Status::belegt) add_(old_H2[i]);
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s3[i] == Status::belegt) add_(old_H3[i]);
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s4[i] == Status::belegt) add_(old_H4[i]);

        //delete temp arrays
        delete[] old_H1;
        delete[] old_H2;
        delete[] old_H3;
        delete[] old_H4;
        delete[] old_s1;
        delete[] old_s2;
        delete[] old_s3;
        delete[] old_s4;
}

template <typename E, size_t N>
void ContDynArray<E,N>::rehash() {
        a1 = random_nmbr(); //random numbers should be huge about 18-20 digits
        a2 = random_nmbr();
        resize(nmax);//call resize to rewrite all elements

}

template <typename E, size_t N>
void ContDynArray<E,N>::remove(const E e[], size_t len) {
        for (size_t i = 0; i < len; ++i) {
                if (member1_(e[i])) {
                        s1[hash1(e[i])] = Status::wiederfrei;
                }else if (member2_(e[i])) {
                        s2[hash2(e[i])] = Status::wiederfrei;
                }else if (member3_(e[i])) {
                        s3[hash3(e[i])] = Status::wiederfrei;
                }else if (member4_(e[i])) {
                        s4[hash4(e[i])] = Status::wiederfrei;
                }
        }
}

template <typename E, size_t N>
bool ContDynArray<E,N>::member1_(const E &e) const {
        if(s1[hash1(e)] == Status::belegt && H1[hash1(e)] == e ) {
                return true;
        }
        else{
                return false;
        }

}

template <typename E, size_t N>
bool ContDynArray<E,N>::member2_(const E &e) const {
        if(s2[hash2(e)] == Status::belegt && H2[hash2(e)] == e ) {
                return true;
        }
        else{
                return false;
        }

}

template <typename E, size_t N>
bool ContDynArray<E,N>::member3_(const E &e) const {
        if(s3[hash3(e)] == Status::belegt && H3[hash3(e)] == e ) {
                return true;
        }
        else{
                return false;
        }

}

template <typename E, size_t N>
bool ContDynArray<E,N>::member4_(const E &e) const {
        if(s4[hash4(e)] == Status::belegt && H4[hash4(e)] == e ) {
                return true;
        }
        else{
                return false;
        }

}

template <typename E, size_t N>
bool ContDynArray<E,N>::member(const E &e) const {
        if(member1_(e) || member2_(e) || member3_(e) || member4_(e)) {
                return true;
        }
        else{
                return false;
        }

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
        o << "H3\n";
        for (size_t i = 0; i < nmax; ++i) {
                o << i << '[' << int(s3[i]) << "] " << H3[i] << '\n';
        }
        o << "H4\n";
        for (size_t i = 0; i < nmax; ++i) {
                o << i << '[' << int(s4[i]) << "] " << H4[i] << '\n';
        }
        return o;
}

template <typename E, size_t N>
size_t ContDynArray<E,N>::apply(std::function<void(const E &)> f, Order order) const {
        size_t rc = 0;
        /*if (order != dontcare) sort();
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
           }*/
        return rc;
}

template <typename E, size_t N>
void ContDynArray<E,N>::sort() const {  // Achtung, O(n*n)
        /*for (size_t i = 0; i < n; ++i) {
                size_t min = i;
                for (size_t j = i+1; j < n; ++j)
                        if (values[min] > values[j]) min = j;
                std::swap(values[min], values[i]);
           }*/
}

 #endif //CONTDYNARRAY_H
