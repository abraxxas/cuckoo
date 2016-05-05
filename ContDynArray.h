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

template <typename E, size_t N=8>
                                class ContDynArray: public Container<E> {
        size_t nmax;
        size_t oldnmax = nmax;//das brauchen wir um über die alten H1 und H2 zu iterieren
        size_t n;
        E * values;

        //http://codereview.stackexchange.com/questions/35220/perfect-hashing-implementation
        //http://liuluheng.github.io/wiki/public_html/Algorithms/Theory%20of%20Algorithms/Hash%20table.html
        //http://users.cis.fiu.edu/~weiss/dsaa_c++4/code/
        const int tMax = 1000; // max number of random walk trials
        int t=0; //number of current walks
        int K; // number of probes
        size_t q=3;//exponent der aktuellen tabellengröße als 2er potenz
        E * H1;
        E * H2;
        E max_v,min_v;

        //vectors are probably not so cool replace with enums!
        /*std::vector<size_t> H1_indices;
           std::vector<size_t> H2_indices;*/
        enum class Status: char { frei, belegt, wiederfrei };
        Status * s1;
        Status * s2;


        std::random_device rd; // only used once to initialise (seed) engine


        size_t a1 = random_nmbr(); //random numbers should be huge about 18-20 digits
        size_t a2 = random_nmbr();

        size_t random_nmbr(){
                std::mt19937 rng(rd()); // random-number engine used (Mersenne-Twister in this case)
                std::uniform_int_distribution<size_t> uni(0,SIZE_MAX); // guaranteed unbiased

                return uni(rng);
        }

        // function for calculation of hash
        size_t hash1(const E& e) const {
                /*std::cout << "a1: " << a1 << "\n";
                   std::cout << "a2: " << a2 << "\n";
                   std::cout << "hashValue(e): " <<hashValue(e) << "\n";
                   std::cout << "a1*hashValue(e): " << a1*hashValue(e) << "\n";
                   std::cout << "CHAR_BIT*sizeof(size_t)-q: " << CHAR_BIT*sizeof(size_t)-q << "\n";
                   std::cout << "((a1*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q)): " << ((a1*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q))  << "\n";*/

                return (a1*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q);
        }
        size_t hash2(const E& e) const {
                return (a2*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q);
        }

        void add_(const E&);

        void resize();
        void rehash();


        bool member1_(const E& e) const;
        bool member2_(const E& e) const;

        void sort() const;
public:
        ContDynArray() : nmax {N}, n {0}, values {new E[this->nmax]}, H1 {new E[this->nmax]}, H2 {new E[this->nmax]},s1 {new Status[this->nmax]()},s2 {new Status[this->nmax]()} { }
        ContDynArray(std::initializer_list<E> el) : ContDynArray() {
                for (auto e: el) add(e);
        }

        virtual ~ContDynArray() {
                delete[] values;
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
        //size_t pos2 = hash2(e);
        std::cout << "adding: " << e << "\n";
        if (s1[pos1] != Status::belegt) {//wenn h1 and entsprechender stelle frei ist
                H1[pos1] = e; //insert elemnt into  H1
                s1[pos1] = Status::belegt;
                std::cout << "We added the value:" << e <<" to H1" << " at the index: " <<  pos1 <<"\n";
        }else{//wenn h1 and entsprechender stelle belegt ist
                E tmp = H1[pos1]; //store the element from H1 in a tmp variable
                if(s2[hash2(tmp)] != Status::belegt) {//h2 and entsprechender stelle ist frei
                        H2[hash2(tmp)] = tmp; //move the conflicting element from tmp to H2
                        H1[pos1] = e; //now insert elemnt into  H1
                        s2[hash2(tmp)] = Status::belegt;
                        std::cout << "We moved the value:" << tmp <<" from H1 at the index:" << pos1 <<  " to H2 at the index: " <<  hash2(tmp) << "\n";
                        std::cout << "and we added the value:" << e <<" to H1" << "\n";
                }
                else{ //both H1 and H2 have something at that their corresponding positions we now need to juggle this arround

                        H1[pos1] = e; //now insert elemnt into  H1
                        E tmp2 = H2[hash2(tmp)]; //store the element from H2 in a tmp variable
                        H2[hash2(tmp)] = tmp; //move the conflicting element from tmp to H2

                        std::cout << "We moved: " << H2[hash2(tmp)] << " from H1 to H2, and added: " << H1[pos1] << " to H1\n";
                        std::cout << "We will now call add again to add: " << tmp2 << " to the hashtable\n";
                        if(t > tMax) {//do we need to rehash?
                                std::cout << "we need to rehash\n";
                                rehash();
                        }
                        t++;
                        add_(tmp2);//call add again to store the element from the tmp2 variable
                }
        }
}

template <typename E, size_t N>
void ContDynArray<E,N>::add(const E e[], size_t len) {
        std::cout << "n: " << n << " len: " << len << " nmax: " << nmax << "\n";
        if (n + len > nmax) {//do we want to add more values than there is currently space? 50%auslastung
                //to be implemented
                std::cout << "50 prozent erreicht wir vergrößern!\n";
                resize();
        }

        for (size_t i = 0; i < len; ++i) { //go through all values we where given
                if (!member(e[i])) { //only execute it he element is neither in H1 nor in H2
                        t = 0;//set current walks to 0
                        add_(e[i]);
                        ++n;
                }else{
                        std::cout << "this element is already a member of H1 or H2 \n";
                }

        }
}

template <typename E, size_t N>
void ContDynArray<E,N>::resize() {
        q++; //increase q by 1
        oldnmax = nmax;//das brauchen wir um über die alten H1 und H2 zu iterieren
        nmax = pow(2,q); //nmax als nächster 2er-potenz setzen
        //save old stuff and allocate space for new stuff
        E * old_H1 = H1;
        E * old_H2 = H2;
        Status * old_s1 = s1;
        Status * old_s2 = s2;
        H1 = new E[nmax]();
        H2 = new E[nmax]();
        s1 = new Status[nmax]();
        s2 = new Status[nmax]();
        //end of save old stuff and allocate space for new stuff

        //write old elements into new hashtables
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s1[i] == Status::belegt) add_(old_H1[i]);
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s2[i] == Status::belegt) add_(old_H2[i]);

        //delete temp arrays
        delete[] old_H1;
        delete[] old_H2;
        delete[] old_s1;
        delete[] old_s2;
}

template <typename E, size_t N>
void ContDynArray<E,N>::rehash() {
        t = 0;//reset t since we have rehashed
        a1 = random_nmbr(); //random numbers should be huge about 18-20 digits
        a2 = random_nmbr();
        //resize();//call resize to rewrite all elements
        oldnmax = nmax;//das brauchen wir um über die alten H1 und H2 zu iterieren
        //save old stuff and allocate space for new stuff
        E * old_H1 = H1;
        E * old_H2 = H2;
        Status * old_s1 = s1;
        Status * old_s2 = s2;
        H1 = new E[nmax]();
        H2 = new E[nmax]();
        s1 = new Status[nmax]();
        s2 = new Status[nmax]();
        //end of save old stuff and allocate space for new stuff

        //write old elements into new hashtables
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s1[i] == Status::belegt) add_(old_H1[i]);
        for (size_t i = 0; i < oldnmax; ++i)
                if (old_s2[i] == Status::belegt) add_(old_H2[i]);

        //delete temp arrays
        delete[] old_H1;
        delete[] old_H2;
        delete[] old_s1;
        delete[] old_s2;
}

template <typename E, size_t N>
void ContDynArray<E,N>::remove(const E e[], size_t len) {
        for (size_t i = 0; i < len; ++i) {
                for (size_t j = 0; j < n; ++j) {
                        if (values[j] == e[i]) {
                                values[j] = values[--n];
                                break;
                        }
                }
        }
}

template <typename E, size_t N>
bool ContDynArray<E,N>::member1_(const E &e) const {
        if(H1[hash1(e)] == e && s1[hash1(e)] == Status::belegt) {
                return true;
        }
        else{
                return false;
        }

}

template <typename E, size_t N>
bool ContDynArray<E,N>::member2_(const E &e) const {
        if(H2[hash2(e)] == e && s2[hash1(e)] == Status::belegt) {
                return true;
        }
        else{
                return false;
        }

}

template <typename E, size_t N>
bool ContDynArray<E,N>::member(const E &e) const {
        if(member1_(e) || member2_(e)) {
                return true;
        }
        else{
                return false;
        }

}

template <typename E, size_t N>
E ContDynArray<E,N>::min() const {
        if (this->empty()) throw ContDynArrayEmptyException();
        E rc = values[0];
        for (size_t i = 1; i < n; ++i) {
                if (rc > values[i]) rc = values[i];
        }
        return rc;
}

template <typename E, size_t N>
E ContDynArray<E,N>::max() const {
        if (this->empty()) throw ContDynArrayEmptyException();
        E rc = values[0];
        for (size_t i = 1; i < n; ++i) {
                if (values[i] > rc) rc = values[i];
        }
        return rc;
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
        if (order != dontcare) sort();
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
        return rc;
}

template <typename E, size_t N>
void ContDynArray<E,N>::sort() const {  // Achtung, O(n*n)
        for (size_t i = 0; i < n; ++i) {
                size_t min = i;
                for (size_t j = i+1; j < n; ++j)
                        if (values[min] > values[j]) min = j;
                std::swap(values[min], values[i]);
        }
}

 #endif //CONTDYNARRAY_H
