#! /bin/sh
echo "Running test 1..." 
python3 kmeans_pp.py 3 333 0 ./tests/input_1_db_1.txt ./tests/input_1_db_2.txt > ./test_output_1.txt
echo "Comparing results..." 
diff ./tests/output_1.txt ./test_output_1.txt
echo "Deleting test_output_1.txt..." 
rm ./test_output_1.txt
echo "Running test 2..." 
python3 kmeans_pp.py 7 0 ./tests/input_2_db_1.txt ./tests/input_2_db_2.txt > ./test_output_2.txt
echo "Comparing results..." 
diff ./tests/output_2.txt ./test_output_2.txt
echo "Deleting test_output_2.txt..." 
rm ./test_output_2.txt
echo "Running test 3..." 
python3 kmeans_pp.py 15 750 0 ./tests/input_3_db_1.txt ./tests/input_3_db_2.txt > ./test_output_3.txt
echo "Comparing results..." 
diff ./tests/output_3.txt ./test_output_3.txt
echo "Deleting test_output_3.txt..." 
rm ./test_output_3.txt
