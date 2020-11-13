### My-DS-CurriculumDesign

SAT-Solver.exe可执行文件使用方法：
1、若选择1进入SAT模块，根据提示输入当前目录下的cnf文件名即可，求解结果会以res文件形式输出到当前目录下。
2、若选择2进入Sudoku模块，先输入将要求解的数独阶数，再输入当前目录下的数度格局txt文件即可进行求解，最终格局会在屏幕中进行打印，各单元格求解结果会以res文件形式输出到当前目录下。


3、Verify_cnf_file文件是用于进行验证cnf算例信息存储到内部结构中的正确与否，验证各子句的内容是否与之前一致。

4、Verify_res_file文件是用于进行验证最终求解结果的正确与否，通过遍历算例中各子句，综合上求解结果，各个文字以True或者False的形式进行输出，便于直接查看是否有不含True的语句，若每个子句至少含有一个True文字，则表示求解结果正确。

代码文件一共有7个，他们相当于分为了四个模块：
5、main.c
6、Cnfparser.h 和 Cnfparser.c
7、Solver.h 和 Solver.c
8、BinaryPuzzle.h 和 BinaryPuzzle.c

用于测试功能的文件：
1、可满足文件：problem12-200.cnf
2、不可满足文件：u-problem7-50.cnf
3、十阶二进制数独初始格局：sudoku_10.txt