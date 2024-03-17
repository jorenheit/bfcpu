### Short Introduction (~30 seconds)
I am a student from V6, and because of my passion in computer science, and electronics, I chose to make a research and development of a processor that runs special machine code. In fact, I thank my physics teacher, Joren Heit, for offering me the participation in this awesome project.

### Brainfuck 
Before I go deeper in the goal of the project, we need some background knowledge. In particular, the understanding of the esoteric programming language, BF. Despite a suspicous name, you can think of this language as a set of specific "commands", that you can run and create different kinds of programs. 

Imagine you woke up in the morning, and you want to shower. What are the specific actions or commands that you perform?
    - You open eyes.
    - You stand up.
    - Carefully go the bathroom.
    - Take clothes off.
    - Start the water flow.
    - Etc.

This mini actions are "commands" that constitute a program, which in our case is "have shower in the morning." 
Similarly, in BF the commands are: >, <, +, -, ., ",", [, ]
Using these commands you can make a program. 

To understand BF, consider a row of so-called cells that can store a number. 

***first animation***

The currently selected cell is pointed at by the arrow.

Now consider this simple program: "++>++>--<+--<+++>--<+++>-++>--<+-<+"
"+" in BF means to add a one to a currently selected cell, so we have 1 in the 1st cell.

[1][][][][][][]

the same goes for the second plus

[2][][][][][][]

">" means to select the next cell, so we are looking at the second cell. 

"-" means to substract a one. So we subtract one from the second cell.

[2][-1][][][][][]

Doing calculations this way, and we will result in the final cell configuration.

On tabels, you have a paper sheet where you can calculate the result yourself during the following 30 seconds. **Time starts now**. Now, on the board you see the animation of one's calculation.

[2][-2][][][][][]
[3][-2][][][][][]
[2][0][][][][][]
[1][0][][][][][]
[0][0][0][0][0][0][0]

So, we got to the point we have started in. If you have this result, be proud because you have done everything correctly.

### Aim 
Now that you understand what BF stands for, we can switch our sight to the computer. In fact, the computer you use can understand only few and really simple operations like ones BF has. This means that whenever you want to start Google, you are on the user-level and you double-click the Google icon to open it up. On the deeper level, computer has the operating system that can translate the Google program in a language that a computer can understand. And once again, these opeations are as simple as increment and go left in BF. Then, the CPU-level comes in, and the CPU can namely run these tremendous amounts of operations in milliseconds. 

The aim of the project is creating CPU that can execute BF code using the simplest electronics components, that you can partly see on the board. 

### Results
Unfortunately, the current state of the CPU is not finished yet. It appeared to be a rather more long-term project, and I cannot demonstrate how BF is executed. However, since according to our desing CPU consists of modules (like memory, calculator-module, etc), some modules are working but are still not connected with each other. For a more in-depth principles of working you can read a mini documentation-book that we have written. And if you seek explanations and observations to what this CPU is already capable of, you can come up to me and discuss it.  

### Process
The process of creating this piece of hardware has been quite difficult yet interesting. 

Firslty, I must have partly read two books about digital and analog electronics to get a basic idea of electronics. Thereafter, I watched series offered by Ben Eater who also created a CPU. Afterwards, it was right the time to think about our own design, circuits, components, and stuff. 

Few progress snapshots are on the board. At some moments, the modules of CPU did not work properly or did not work at all, and it required some fixing, or debugging. On some occasions, I was involved in this process of debugging for two weeks. 

### Next steps
I am determined to lead this project till the end. So, my next steps are creating the rest of the modules, and connecting them, then testing their work, and release the first version of the CPU. If you would like to follow the course of action, you can visit this webpage that contains the software-side of the project and the documentation that I try keeping up-to-date.

### Goodbye
Well, that was it. Hope you liked this presentation and enjoy the rest of the evening! 