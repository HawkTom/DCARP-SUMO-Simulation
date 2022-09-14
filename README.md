# DCARP-SUMO-Simulation
Benchmarking Dynamic Capacitated Arc Routing Algorithms Using Real-World Traffic Simulation. 

- This work has been accepted by IEEE CEC 2022. Its proceeding is still on the way. 
- The paper of this work is avaliable online. [Pdf](https://www.cs.bham.ac.uk/~minkull/publications/TongCEC2022.pdf) 

```tex
@INPROCEEDINGS{tong2022sumo,  
    author={Tong, Hao and Minku, Leandro L. and Menzel, Stefan and Sendhoff, Bernhard and Yao, Xin},  
    booktitle={2022 IEEE Congress on Evolutionary Computation (CEC)},   
    title={Benchmarking Dynamic Capacitated Arc Routing Algorithms Using Real-World Traffic Simulation},   
    year={2022},  
    volume={},  
    number={},  
    pages={1-8},  
    doi={10.1109/CEC55065.2022.9870399}
}
```



## Motivations

- We use SUMO as a platform including real traffic data to evaluate the performance of proposed DCARP optimisation algorithm. 

- The DCARP (Dynamic Capacitated Arc Routing Algorithms) scenarios are constructed in the SUMO.  (Below is the paper about DCARP.)

  ```
  @article{tong2022dcarp,
    year = {Early Access, 2022. Doi: 10.1109/TEVC.2022.3147509},
    publisher = {Institute of Electrical and Electronics Engineers ({IEEE})},
    author = {Hao Tong and Leandro L. Minku and Stefan Menzel and Bernhard Sendhoff and Xin Yao},
    title = {A Novel Generalised Meta-Heuristic Framework for Dynamic Capacitated Arc Routing Problems},
    journal = {{IEEE} Transactions on Evolutionary Computation}
  }
  ```

## Usage

*Requirments:* 

- Install SUMO in your machine 
- An optimizer for DCARP optimization (I have given one compiled exe in this repo. You can also compile your own optimizer. The folder `optimization` give the source code of our interface)

*Explaination:* 

- `Dublin` traffic data is placed in the `scenario` folder 
- DCARP scenario constructed by ourselves are in `dcarp` folder
- Map data of Dublin city is placed in the `traffic` folder. It is just for convenient usage. 
- Folder `xml` is used for simulation results 

*Run*:

`python main.py` 

## Paper results

- **Scenarios generated:** in the `dcarp` folder in this repo. 
- **Instances resulted:**  Download from this [link](https://drive.google.com/file/d/1hemgVzAcRpOf6U4ARoQQkuUpYUvXlN5o/view?usp=sharing) because it is too big. 



## Problem feedback

For any problems, you can raise up your problem in the issue of this repo. I will help you with my best. 

 
