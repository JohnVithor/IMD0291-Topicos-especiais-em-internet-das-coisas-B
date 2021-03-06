#!/bin/sh

#cd "Valor de PI"/
#python3 comparator.py args_teste.json 10 2 1

#cd ..

#cd Trapezios/

#python3 comparator.py args_teste.json 10 2 1

#cd ..

cd "Valor de PI"/

mv serial_results.json serial_results1.json
mv parallel_results.json parallel_results1.json

python3 metrics.py args_teste.json serial_results1.json parallel_results1.json

mv metrics_a.json metrics_1a.json
mv metrics_b.json metrics_1b.json

python3 graphics.py args_teste.json metrics_1a.json

mv eficiencias_problem_size.png eficiencias_problem_size_1a.png
mv speedups.png speedups_1a.png
mv eficiencias_cores.png eficiencias_cores_1a.png

python3 graphics.py args_teste.json metrics_1b.json

mv eficiencias_problem_size.png eficiencias_problem_size_1b.png
mv speedups.png speedups_1b.png
mv eficiencias_cores.png eficiencias_cores_1b.png

cd ..

cd Trapezios/

mv serial_results.json serial_results1.json
mv parallel_results.json parallel_results1.json

python3 metrics.py args_teste.json serial_results1.json parallel_results1.json

mv metrics_a.json metrics_1a.json
mv metrics_b.json metrics_1b.json

python3 graphics.py args_teste.json metrics_1a.json

mv eficiencias_problem_size.png eficiencias_problem_size_1a.png
mv speedups.png speedups_1a.png
mv eficiencias_cores.png eficiencias_cores_1a.png

python3 graphics.py args_teste.json metrics_1b.json

mv eficiencias_problem_size.png eficiencias_problem_size_1b.png
mv speedups.png speedups_1b.png
mv eficiencias_cores.png eficiencias_cores_1b.png
