from manim import *
from copy import deepcopy

class BF_Animation(Scene):
    def bf_setup(self):
        # program to visualize
        # self.program = "++>++>--<+--<+++>--<+++>-++>--<+-<+"
        self.program = "+>+>+>-<"
        
        # number of BF-operations
        self.N = len(self.program)

        # create N cells
        self.cell_color = WHITE
        self.opacity  = 0.5
        self.cells = VGroup(*[Square(side_length=1).set_color(self.cell_color, self.opacity) for _ in range(self.N)]) 

        # initialize values of each cell
        self.values = VGroup(*[Integer(0) for _ in range(self.N)])

        # the index of the selected cell
        self.selected_cell = 0
        
        # place cells in a sequence
        for i, cell in enumerate(self.cells):
            cell.move_to(RIGHT * i + 6 * LEFT)

        # selection arrow
        self.arrow = Arrow(ORIGIN, UP)
        self.update(0)

    def construct(self):
        self.bf_setup()

        current_operation_text = Text("Current operation:")
        current_operation_text.move_to(UP + 2 * LEFT)
        self.add(current_operation_text)

        # place decimals in cells
        for i, (cell, value) in enumerate(zip(self.cells, self.values)):
            value.move_to(cell)

        # animate cells and values
        self.play(AnimationGroup(
            *[Create(cell) for cell in self.cells],
            *[Create(value) for value in self.values],
            Create(self.arrow),
        ))

        for operation in self.program:
            self.apply_bf_operation(operation)

    def update(self, delta):
        self.selected_cell += delta
        self.arrow.next_to(self.cells[self.selected_cell], DOWN)
    
    def apply_bf_operation(self, operation):
        # set current operation text
        text_operation = Text(f"{operation}")
        text_operation.move_to(UP + 2 * RIGHT)
        self.play(FadeIn(text_operation))

        if operation == '+':
            self.play(ChangeDecimalToValue(self.values[self.selected_cell], self.values[self.selected_cell].get_value() + 1))
        elif operation == '-':
            self.play(ChangeDecimalToValue(self.values[self.selected_cell], self.values[self.selected_cell].get_value() - 1))
        elif operation == '<':
            self.update(-1)
        elif operation == '>':
            self.update(1)
        
        self.play(FadeOut(text_operation))
