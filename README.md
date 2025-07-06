
## ***CH83MU - Basic Chip8 Emulator Guide	
	
	Marcos Oduardo Castillo - 7 julio 2025

Uso del ejecutable: ./chip8.elf Escala Delay ROMfilename
Se recomienda una escala de 10 y un delay entre 3 y 4 

------------------------------------------------------------------------

Este documento se trata de una memoria que detalla el proceso y realización de un emulador para el sistema Chip8, que surgió a mediados de los 80. Elegí Chip8 debido a que muchas personas consideran que tiene un repertorio de instrucciones y unas especificaciones ideales para utilizarlo como primer proyecto en el contexto de la emulación. Existe documentación muy amplia sobre este sistema y sus características. Al final del documento se incluyen todas las fuentes en las que he basado mi implementación, recomendando especialmente la guía de Austin Morlan, la cual ha resultado esencial para la realización de este proyecto. Frente al código que anteriormente he mencionado, he realizado modificaciones en el encapsulamiento de datos y su inicialización.

El código que he escrito está totalmente comentado con explicaciones, aclaraciones de funciones y decisiones de diseño, con el objetivo de que sirviese como guía para mas gente que se interesase por el funcionamiento de la aplicación o quiera tomarlo como base para su propio código.

------------------------------------------------------------------------

Chip8 se trata de un lenguaje interpretado, no de un hardware real. Tiene una maquina virtual donde corren todos los juegos programados con Chip8, se hizo así para conseguir programar facilmente juegos en los computadores  de la época.

La maquina que emularemos será esta VM Chip8, con un repertorio de instrucciones simulado, es decir, un sistema capaz de ejecutar el bytecode que ejecuta la maquina virtual. Vamos a crear este "interprete", y lo trataremos como un emulador, en lenguaje C++

0x , $ <-- indican que el numero que sigue es un hexadecimal. Podemos tomar como ejemplo la instrucción $7522 del repertorio de Chip8, que indica que añadiremos 22 al registro 5. De esta manera, la CPU recoge las instrucciones que genera la ejecución del programa en memoria y las ejecuta paso a paso. Esta instrucción en assembler seria algo como ADD r5, $22

Un emulador lee el código maquina original compilado para el sistema que emula, lo decodifica y ejecuta los efectos de la instrucción en el entorno del hardware simulado (CPU, registros, memoria...) . El archivo ROM contiene todas las instrucciones maquina del programa, el emulador las lee y simula la maquina original. Al emular maquinas mas complejas hay que hacer mas trabajo para emular otros componentes como la tarjeta de sonido, el procesador gráfico...Chip8 es un sistema simple con un instruction set reducido, una apariencia gráfica unicamente de pixeles simples y unos buzzers por sonidos, por lo cual es una buena máquina para trabajar los conceptos mas básicos de la emulación.

Nota: Chip8 tiene algunas diferencias notables con la manera general de percibir un lenguaje interpretado como Python. Python es un lenguaje de alto nivel, que utiliza un interprete el cual crea el bytecode a partir de este codigo fuente y lo ejecuta en la PVM (python virtual machine). Chip8 tiene su propio lenguaje en el que los programadores codifican el juego y la maquina virtual ejecuta directamente el código, sin necesidad de un compilador. Uno es de alto nivel y otro de bajo nivel.

### Descripción del Chip8:

**16 registros de 8 bits** --> Un registro es una sección física de la CPU reservada a guardar datos, y con esos datos realiza operaciones. Estos 16 registros se enumeran de V0 a VF, y pueden contener datos desde el valor 0x00 al 0xFF. El registro VF se utiliza como una flag para información de las operaciones realizadas. En decimal, 0xFF seria 255

**4KB de memoria**(ROM Y RAM) --> Aquí se guardan las instrucciones que ha de ejecutar la CPU, así como información relativamente a largo plazo, corto plazo...nos referimos a diferentes segmentos y puntos en la memoria con direcciones o "addresses"

Tiene 4096 Bytes de memoria, por lo que el valor de las direcciones va de 0x000 a 0xFFF (16 ^ 3, se necesitan 12 bits como mínimo para codificar las direcciones). El espacio de memoria se divide de la siguiente forma:

0x000 - 0x1FF -> Reservado para el interprete de Chip8, en este caso, nuestro emulador lo substituirá

0x050 - 0x0A0 -> Reservado para 16 sprites de caracteres de 0 a F que habremos de insertar manualmente en memoria debido a que los programas accederán a esas posiciones a buscarlos

0x200 - 0xFFF -> Donde se carga la ROM a ejecutar (sus instrucciones). Todo espacio que quede despues de la ROM es libre.

**Index register (16 bits)** -> Registro especial que guarda direcciones de memoria para usar en operaciones

**Program Counter (16 bits)** -> Contiene la dirección de la siguiente instrucción a ejecutar. Como las instrucciones de Chip8 ocupan 2 bytes, primero hacemos fetch al primer byte, luego hacemos fecth al byte PC + 1, los unimos y finalmente añadimos 2 al PC para continuar a la siguiente instrucción. Antes de guiaejecutar cada instrucción el PC deberá modificarse a sus siguiente valor ya que puede ser que esa instruccion vaya a modificar su valor.

**Stack de 16 niveles** -> Un stack, pila, permite a la CPU ejecutar instrucciones CALL, donde el PC de la siguiente instrucción a ejecutar deberá guardarse en la pila y deberá saltar a la dirección de memoria que le indica la instrucción. Al llegar a la instruccion RET, la CPU debe hacer un pop del valor del PC guardado en la pila y proseguir la ejecución anterior. 16 niveles significa que se pueden hacer 16 llamadas a funciones dentro de otras funciones.

**Stack Pointer de 8 bits** -> indica cual es la dirección en la pila del valor que acabamos de guardar. El stack es un array de 16 valores, así que con un solo byte nos sirve. El sp será un indice de ese array.

**Delay timer de 8 bits & Sound Timer 8 bits**-> Timers simples, el timer si está en 0 no hará nada pero si tiene cualquier otro valor decrementará su tiempo cada 60Hz del reloj, igual el timer del sonido pero emitirá ruido cuando no sea 0.

**16 input keys** las cuales mappearemos de la siguiente manera:

```
Keypad       Keyboard
+-+-+-+-+    +-+-+-+-+
|1|2|3|C|    |1|2|3|4|
+-+-+-+-+    +-+-+-+-+
|4|5|6|D|    |Q|W|E|R|
+-+-+-+-+ => +-+-+-+-+
|7|8|9|E|    |A|S|D|F|
+-+-+-+-+    +-+-+-+-+
|A|0|B|F|    |Z|X|C|V|
+-+-+-+-+    +-+-+-+-+
```

**Memoria 64x32 de display monocromático** -> Es un memory buffer que guarda los gráficos que se van a mostrar. Los pixeles pueden estar ON o OFF, solo muestran 2 colores. (Esta operación probablemente sea la mas complicada de realizar en el emulador.)

La manera en la que funciona es que itera todos los pixeles del sprite y ejecuta una operación XOR entre el Pixel Sprite y el Pixel DIsplay. XOR hace que solo cuando sean diferentes, el output sea 1, asi que si por ejemplo queremos hacer que un sprite se mueva de sitio y sabemos que el sprite esta dibujado en (X,Y), podemos ejecutar un comando de dibujado en (X,Y) del mismo sprite para borrarlo (XOR vera que ambos son 1 y devolverá 0 en todas las posiciones) para despues escribirlo en (X+1, Y+1), por ejemplo. 

### Programación del emulador:

El programa será elaborado en el lenguaje C++

En primer lugar, creamos un Makefile para el proyecto y despues creamos un main simple para probar el funcionamiento del comando make.

Después tenemos que definir todos los componentes hardware descritos anteriormente en una clase llamada Chip8. Un objeto de clase Chip8 será el elemento que simulará todo el sistema, con métodos para ejecutar el funcionamiento.

```C++
#include <cstdint>

class Chip8 //Basic design of the system

{

public:
void LoadROM(char const* filename);

  
private:
uint8_t V[16]; //System registers
uint8_t MEM[4096]; //Memory address designed to each byte of the MM
uint16_t I_REG; //index register
uint16_t PC; //Program Counter
uint16_t STACK[16];
uint8_t SP;
uint8_t D_TIMER;
uint8_t S_TIMER;
uint8_t KEYPAD[16];
uint32_t MEM_BUFFER[64*32];
uint16_t OPCODE; //2 Bytes opcodes

};
```

Hace falta definir los métodos mencionados, a continuación está el código de la función LoadROM:

```C++
#include "chip8.h"
#include <fstream>

const unsigned int START_ADDRESS = 0x200;

  
Chip8::Chip8()
{
PC = START_ADDRESS;
}

  
void Chip8::LoadROM(char const* filename)
{
//open file as binary file, moves pointer at the end of file
std::ifstream file(filename, std::ios::binary | std::ios::ate);
  

if(file.is_open())
{
//calculates the size of the file and creates a buffer for allocating its contents
std::streampos size = file.tellg(); //streampos = flexible data type for pointer positions in streams. tellg() returns current position (at the end, size)
char* buffer = new char[size]; //size is unknow in compilation time -> we need to use pointers and dynamic memory

  
file.seekg(0, std::ios::beg); //changes current position from begining (0 is the offset, and we don't need one)
file.read(buffer, size); //reads al the data from the starting position to the indicated size into the buffer
file.close();

//LOAD the ROM contents into the system's memory from the adress 0x200
for (std::size_t i = 0; i < size; i++)
{
MEM[START_ADDRESS + i] = buffer[i];
}

delete[] buffer;
}

}

```

También tenemos que inicializar las fuentes en memoria. Las fuentes se representan con sprites de 8x5 bits, donde cada bit representa un pixel y su valor 1 o 0 define si se enciende o se apaga.

El caracter A, por ejemplo:

```
11110000
10010000
11110000
10010000
10010000
```

Vendria a ser codificado con los valores 0xF0, 0x90, 0xF0, 0x90, 0x90. A continuación, el array que define los valores de las 16 fuentes en memoria:



Entonces podemos incializarlos en memoria a partir de la dirección 0x050 en memoria

```C++

const unsigned int FONTSET_SIZE = 80; //each font in memory is defined by 5 set>
const unsigned int FONTSET_START_ADDRESS = 0x050;


uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
{
    PC = START_ADDRESS;

    for (unsigned int i = 0; i < FONTSET_SIZE; i++)
    {
        MEM[FONTSET_START_ADDRESS + i] = fontset[i];
    }
};

```

Hay que diseñar un sistema que nos permita generar numeros aleatorios (RNG), fisicamente podria hacerse utilizando un Chip RNG o leyendo un pin con mucho ruido, pero como nuestro sistema es software utilizaremos las funciones Random que nos ofrece C++ (usamos las clases random y chrono de C++ en el .h)

```C++
std::default_random_engine randGen;
std::uniform_int_distribution<uint8_t> randByte {0,255}; //random class object, random value between 0 and 255 when given a randGen
```

y en el .cpp

```C++

Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) //constructor, also initializes randGen seed with internal clock
{
	[...]
}

```

uniform_int_distribution es una clase de la libreria random que nos permite conseguir un valor entre los que hemos definido con la misma probablidad de aparecer. Esto lo consigue randByte al proporcionarle como parametro del constructor randGen (randByte(randGen)), el cual usa una seed (en este caso el valor actual del reloj interno) para generar un número aleatorio crudo, el randByte transforma en un valor de su rango.

**- Implementación de las instrucciones**

El sistema tiene 34 instruciones a implementar, cada una es una función que hay que implementar en nuestro codigo. A continuación, se indica el opcode, su función y el codigo que define su funcionamiento. La implementación que seguiremos será la que viene definida por Cowgod (http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)

- 00E0 -> CLS -> Clear Display
- 00EE -> RET
- 1nnn -> JMP addr -> jumps to NNN
- 2nnn -> CALL addr -> calls subroutine at NNN 
- 3xkk -> SE Vx, byte -> Skips instruction if Vx = kk
- 4xkk -> SNE Vx, byte -> Skips instruction if Vx != kk
- 5xy0 -> SE Vx, Vy -> Skips instruction if Vx = Vy
- 6xkk -> LD Vx, byte -> Set Vx = kk
- 7xkk -> ADD Vx, byte -> Set Vx = Vx + 1
- 8xy0 -> LD Vx, Vy -> Set Vx = Vy
- 8xy1 -> OR Vx, Vy -> Set Vx = Vx OR Vy
- 8xy2 -> AND Vx, Vy -> Set Vx = Vx AND Vy
- 8xy3 -> XOR Vx, Vy -> Set Vx = Vx XOR Vy
- 8xy4 -> ADD Vx, Vy -> Set Vx = Vx + Vy, if result > 255 VF is set to 1. Stores lower 8 bits
- 8xy5 -> SUB Vx, Vy -> Set Vx = Vx - Vy, if Vx > Vy VF = 1,  otherwise 0
- 8xy6 -> SHR Vx -> Set Vx SHR 1, if the least significant bit is 1, VF = 1. (saved in VF) Then Vx / 2 
- 8xy7 -> SUBN Vx, Vy -> Set Vx = Vy - Vx, If Vy > Vx then VF = 1
- 8xyE -> SHL Vx {, Vy} -> Set Vx = Vx SHL 1
- 9xy0 -> SNE Vx, Vy -> Skips instruction if Vx != Vy
- Annn -> LD I, addr -> Set I = nnn
- Bnnn -> JP v0, addr -> Jumps to nnn + V0
- Cxkk -> RND Vx, byte -> Vx = random byte AND kk
- Dxyn -> DRW Vx, Vy, nibble -> Displays n byte sprite starting at memory location I (index) at (Vx, Vy). Set VF = collision
- Ex9E -> SKP Vx ->  Skip next instruction if key with Vx value is pressed
- ExA1 -> SKNP Vx -> Skip next instruction if key with Vx value is not pressed
- Fx07 -> LD Vx, DT -> Set Vx = delay timer value
- Fx0A -> LD Vx, K -> Wait for a key press, store the value of the key in LD
- Fx15 -> LD DT, Vx -> Set delay timer = Vx
- Fx18 -> LD ST, Vx -> Set sound timer = Vx
- Fx1E -> ADD I, Vx -> Set I = I + Vx
- Fx29 -> LD F, Vx -> Set I = location of sprite for digit Vx
- Fx33 -> LD B, Vx -> Store BCD representation of Vx in memory locations I, I+1, I+2 
- Fx55 -> LD [I], Vx -> Store registers V0 through Vx in memory starting at location I
- Fx65 -> LD Vx, [I] -> Read registers V0 through Vx from memory starting at location I

El código para estas funciones puede encontrarse en el archivo chip8.cpp.

**- Tabla de punteros de instrucciones** 

La forma mas fácil para decodificar un opcode seria un switch-case, pero para implementar una estructura mas adecuada y escalable usaremos un array de apuntadores a funciones donde el opcode es un indice dentro de esa tabla

Separaremos los opcodes por su primer digito, viendo si se repite en otros opcodes o no. Para los que su primer digito (de $0 a $F) es único no necesitaremos mas que ese dígito para identificarlos, pero para los que su primer digito se repite en otros opcodes necesitaremos una tabla extra para indexar el resto de diferencias.

```C++
void Chip8::Table0()
{
    ((*this).*(table0[opcode & 0x000Fu])) (); // Insane sentence, but this executes the function returned by the member function
                                              // pointer of the current class object. The () is what executes the function,
                                              // the .* is a operand to get the pointer of the function member, but doesn't execute it.

                                              //An equivalent line of code would be (this->*table0[opcode & 0x000Fu]) ();
};

void Chip8::Table8()
{
    (this->*table8[opcode & 0x000Fu]) (); //All these functons serve as the second table for the repeated opcode nums.
};

void Chip8::TableE()
{
    (this->*tableE[opcode & 0x000Fu]) ();
};

void Chip8::TableF()
{
    (this->*tableF[opcode & 0x00FFu]) ();
};

void Chip8::OP_NULL()
{

};


Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) //constructor, also initializes randGen seed with internal clock
{
    PC = START_ADDRESS;

    for (unsigned int i = 0; i < FONTSET_SIZE; i++)
    {
        MEM[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    //POINTER TABLE!! 
    
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF; //master table is full, now we have to assign secondary tables (which won't be full)

    for (size_t i = 0; i <= 0xE ; i++)
    {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }

    for (size_t i = 0; i <= 0x65 ; i++)
    {
        tableF[i] = &Chip8::OP_NULL;
    }

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;

    



};
```

Después de definir la tabla de funciones hay que programar la lógica de funcionamiento de las instrucciones. Se puede consultar como se han programado en el archivo .cpp de la clase Chip8.

**- Implementación de los ciclos CPU**

Una vez definidas todas las instruciones tenemos que programar lo que será 1 ciclo de la CPU. Este ciclo se compone de 3 fases basicas, Fetch, decode y execute. La CPU consigue los 2 bytes que componen la instruccion en la posicion de memoria PC y PC + 1, La decodifica usando la tabla de funciones y a su vez la ejecuta. Una vez esto haya sido realizado, si los registros delay o timer tienen valores positivos, se decrementan por 1. No nos preocuparemos por cumplir los 60hz a los que se decrementan, ya que esto depende de la velocidad de la CPU.

```C++

void Chip8::Cycle()
{
    opcode = (MEM[PC] << 8u | MEM[PC + 1]); //First bit gets shifted to upper OPCODE half

    PC += 2;

    (this->*table[(opcode & 0xF000 >> 12u)]) ();

    if (D_TIMER > 0)
    {
        D_TIMER -= 1;
    }

    if (S_TIMER > 0)
    {
        S_TIMER -= 1;
    }
}
```


**- Implementación de la Platform Layer con SDL**

SDL (Simpe DirectMedia Layer) es una API multiplataforma que actúa como capa de abstracción en diversas funciones que requieren del uso de APIs especificas del SO. Nos permite hacer acciones como crear ventanas o recoger inputs sin tener que programar especificamente para un sistema concreto, convirtiendo nuestra aplicacion en multiplataforma.

En general, la capa de plafatorma de un proyecto es el módulo que se encarga de interactuar con el Sistema Operativo. Esto incluye dibujar graficos, procesar inputs, procesar audio, procesar multithreading...En nuestro caso, como es un emulador antiguo, el procesado de graficos 2D es lo suficientemente simple como para ser realizado manualmente sin necesidad de ninguna API adicional. Es decir, si que utilizamos SDL para gestionar la comunicación con la pantalla del host donde se ejecuta, pero la lógica del dibujado del Memory Buffer que será escrito en pantalla es gestionado por el core del Emulador.

Como es un emulador simple, solo tendremos dos capas, Emulator Core y Platform Layer, pero podrían añadirse mas, como UI layer para una interfaz mas moderna o Debugging Layer. Toda la información de la implementacion de la Platform Layer está comentada en el codigo, especificamente en la cabezera platform.h

**- Implementación del main del programa**

En el main del programa definiremos que se han de indicar 3 argumentos para la ejecucion del programa: Escala (hay que reescalar los 64x32 pixeles de Chip8 a monitores modernos), cierto tiempo de delay en cada ciclo de ejecución y el nombre de la ROM de donde vamos a leer los opcodes. En el main incializaremos la plataforma con SDL y una instancia del emulador Chip8, con un bucle while infinito donde se ejecuta un ciclo de la CPU y se renderiza la textura de la ventana con la información del Memory Buffer (próximo frame a escribir)

```C++
#include <iostream>
#include "chip8.h"
#include "platform.h"

int main(int argc, char** argv)
{
    if (argc != 4){
        std::cerr << "Uso: " << argv[0] << "<Escala> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    int videoScale = std::stoi(argv[1]); //we need to scale 64x32 pixels to an adequate number for modern screens
    int cycleDelay = std::stoi(argv[2]);
    char const* romFilename = argv[3];

    Platform platform("Ch83mu", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 Emulator;

    Emulator.LoadROM(romFilename);

    int videoPitch = sizeof(Emulator.MEM_BUFFER[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit)
    {
        quit = platform.ProcessInput(Emulator.KEYPAD);
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count(); //delay time set up for the clock
        if (dt > cycleDelay)
		{
			lastCycleTime = currentTime;

			Emulator.Cycle();

			platform.Update(Emulator.MEM_BUFFER, videoPitch);
		}

    }

    return 0;
}
```

Y finalmente, solo queda compilar todo el programa (se recomienda el makefile proporcionado para entornos LInux) y habremos construido un emulador funcional para archivos Chip8.

-----------------------------------------------------------------------

## ***Bibliografia

[-> Building a CHIP-8 Emulator [C++] - Austin Morlan](https://austinmorlan.com/posts/chip8_emulator/)

[-> Chip8 emulator in C++ - tmvtech](https://www.tmvtech.com/chip8-emulator-in-cpp/)

[-> Chip8 Technical Reference v1.0 - Cowgod](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.1)

