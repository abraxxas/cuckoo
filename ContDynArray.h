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



 class ContDynArrayEmptyException : public ContainerException {
 public:
   virtual const char * what() const noexcept override { return "ContDynArray: empty"; }
 };

 template <typename E, size_t N=7>
 class ContDynArray : public Container<E> {
   size_t nmax;
   size_t n;
   E * values;

   //http://codereview.stackexchange.com/questions/35220/perfect-hashing-implementation
   //http://liuluheng.github.io/wiki/public_html/Algorithms/Theory%20of%20Algorithms/Hash%20table.html
   //http://users.cis.fiu.edu/~weiss/dsaa_c++4/code/
   const int tMax = 10000; // max number of random walk trials
   int K; // number of probes
   size_t number_max; // number of max elements
   size_t number; // number of current elements
   size_t q=3;//exponent der aktuellen tabellengröße als 2er potenz
   E * H1,H2;
   size_t * H1_indices,H2_indices;

   std::random_device rd;     // only used once to initialise (seed) engine


   size_t a1 = random_nmbr(); //random numbers should be huge about 18-20 digits
   size_t a2 = random_nmbr();

   size_t random_nmbr(){
     std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
     std::uniform_int_distribution<size_t> uni(0,SIZE_MAX); // guaranteed unbiased

     return uni(rng);
   }

   // function for calculation of hash
   size_t hash1(const E& e) const {
     std::cout << "a1: " << a1 << "\n";
     std::cout << "a2: " << a2 << "\n";
     std::cout << "hashValue(e): " <<hashValue(e) << "\n";
     std::cout << "a1*hashValue(e): " << a1*hashValue(e) << "\n";
     std::cout << "CHAR_BIT*sizeof(size_t)-q: " << CHAR_BIT*sizeof(size_t)-q << "\n";
     std::cout << "((a1*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q)): " << ((a1*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q))  << "\n";

     return (a1*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q);
   }
   size_t hash2(const E& e) const {
     return (a2*hashValue(e))>>(CHAR_BIT*sizeof(size_t)-q);
   }


   void sort() const;
 public:
   ContDynArray() : nmax{N}, n{0}, values{new E[this->nmax]} { }
   ContDynArray(std::initializer_list<E> el) : ContDynArray() { for (auto e: el) add(e); }

   virtual ~ContDynArray() { delete[] values; }

   using Container<E>::add;
   virtual void add(const E e[], size_t len) override;

   using Container<E>::remove;
   virtual void remove(const E e[], size_t len) override;

   virtual bool member(const E& e) const override;
   virtual size_t size() const override { return n; }

   virtual E min() const override;
   virtual E max() const override;

   virtual std::ostream& print(std::ostream& o) const override;

   virtual size_t apply(std::function<void(const E&)> f, Order order = dontcare) const override;
 };

 template <typename E, size_t N>
 void ContDynArray<E,N>::add(const E e[], size_t len) {
   for (size_t i = 0; i < len; ++i)
       hash1(e[i]);
   /*if (n + len > nmax) {
     auto newnmax = nmax;
     E * newvalues = nullptr;
     while (n + len > newnmax) newnmax = (newnmax*12)/10 + 2;
     newvalues = new E[newnmax];
     for (size_t i = 0; i < n; ++i)
       newvalues[i] = values[i];
     delete[] values;
     values = newvalues;
     nmax = newnmax;
   }
   for (size_t i = 0; i < len; ++i)
     if (!member(e[i]))
       values[n++] = e[i];*/
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
 bool ContDynArray<E,N>::member(const E& e) const {
   for (size_t i = 0; i < n; ++i) {
     if (values[i] == e) return true;
   }
   return false;
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
   o << "ContDynArray [ n=" << n << " nmax=" << nmax << " values=";
   for (size_t i = 0; i < n; ++i) o << ' ' << values[i];
   o << " ]";
   return o;
 }

 template <typename E, size_t N>
 size_t ContDynArray<E,N>::apply(std::function<void(const E&)> f, Order order) const {
   size_t rc = 0;
   if (order != dontcare) sort();
   try {
     if (order == descending) {
       for (size_t i = n; i--;) {
         f(values[i]);
         ++rc;
       }
     } else {
       for (size_t i = 0; i < n; ++i) {
         f(values[i]);
         ++rc;
       }
     }
   } catch (...) {}
   return rc;
 }

 template <typename E, size_t N>
 void ContDynArray<E,N>::sort() const { // Achtung, O(n*n)
   for (size_t i = 0; i < n; ++i) {
     size_t min = i;
     for (size_t j = i+1; j < n; ++j)
       if (values[min] > values[j]) min = j;
     std::swap(values[min], values[i]);
   }
 }

 #endif //CONTDYNARRAY_H
