komenda do uruchamiania z glownego folderu projektu:

./build/pea_zad1 config/nazwa_pliku.txt

oczywiscie w build cmake .. po czym make po czym cd .. i uruchomienie z glownego folderu projketu

pliki csv sie dopisuja wiec jak cos zeby byly stale dla danej proby to lepiej usunac i zrobic nowy pomiar

---
Testy pojedyncze
---
   -test_read

   ./build/pea_zad1 config/00_test_read_att48.txt
   
   -check_opt

   ./build/pea_zad1 config/01_check_opt_att48.txt
   

-single RAND

   ./build/pea_zad1 config/02_single_rand_att48.txt
  

 -single NN

   ./build/pea_zad1 config/03_single_nn_att48.txt
  
-single RNN dla TSP

   ./build/pea_zad1 config/04_single_rnn_att48.txt

-single RNN dla ATSP

   ./build/pea_zad1 config/05_single_rnn_br17.txt

----
Benchmark heurystyk
---

TSP
   
   ./build/pea_zad1 config/06_heuristics_tsp.txt

ATSP

   ./build/pea_zad1 config/07_heuristics_atsp.txt
   
----
Benchmark brute force
---
TSP

   ./build/pea_zad1 config/08_bf_tsp.txt

   ATSP

   ./build/pea_zad1 config/09_bf_atsp.txt
   