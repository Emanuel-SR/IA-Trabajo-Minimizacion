#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <GL/glut.h>

using namespace std;

const int TAM_POBLACION = 10;
const int GENERACIONES = 100;
const double TASA_MUTACION = 0.05;   // 5% de probabilidad de mutación
const double TASA_CRUZA = 0.85;      // 85% de probabilidad de cruzamiento

enum Objetivo { MINIMIZAR, MAXIMIZAR };
const Objetivo OBJETIVO_ACTUAL = MINIMIZAR; // Manual: MINIMIZAR o MAXIMIZAR

struct Individuo {
    int x;
    int y;
    double fitness;
};

vector<Individuo> poblacion(TAM_POBLACION);
vector<double> historialMejores;
vector<double> historialPromedios;
Individuo elMejorDeTodos;

// Fitnes
double evaluarFuncion(int x, int y) {
    return pow(x, 2) - (2 * x * y) + pow(y, 2);
}

bool esMejor(double fitness1, double fitness2) {
    if (OBJETIVO_ACTUAL == MINIMIZAR) {
        return fitness1 < fitness2;
    }
    else {
        return fitness1 > fitness2;
    }
}

void inicializarPoblacion() {
    for (int i = 0; i < TAM_POBLACION; i++) {
        poblacion[i].x = rand() % 128; // 7 bits (0 a 127)
        poblacion[i].y = rand() % 64;  // 6 bits (0 a 63)
        poblacion[i].fitness = evaluarFuncion(poblacion[i].x, poblacion[i].y);
    }
}

void operarAlgoritmoGenetico() {
    cout << "==================================================" << endl;
    cout << "        PROGRESO DE LAS GENERACIONES             " << endl;
    cout << "        Objetivo: " << (OBJETIVO_ACTUAL == MINIMIZAR ? "MINIMIZAR" : "MAXIMIZAR") << endl;
    cout << "==================================================" << endl;

    for (int gen = 0; gen < GENERACIONES; gen++) {
        double sumaFitness = 0;
        Individuo mejorActual = poblacion[0];

        for (int i = 0; i < TAM_POBLACION; i++) {
            poblacion[i].fitness = evaluarFuncion(poblacion[i].x, poblacion[i].y);
            sumaFitness += poblacion[i].fitness;

            if (esMejor(poblacion[i].fitness, mejorActual.fitness)) {
                mejorActual = poblacion[i];
            }
        }

        double promedioActual = sumaFitness / TAM_POBLACION;
        historialMejores.push_back(mejorActual.fitness);
        historialPromedios.push_back(promedioActual);

        cout << "Gen " << gen << " -> Mejor Fitness: " << mejorActual.fitness
            << " | Promedio: " << promedioActual
            << " (Mejor X: " << mejorActual.x << ", Y: " << mejorActual.y << ")" << endl;

        if (gen == 0 || esMejor(mejorActual.fitness, elMejorDeTodos.fitness)) {
            elMejorDeTodos = mejorActual;
        }

        vector<Individuo> nuevaPoblacion(TAM_POBLACION);
        nuevaPoblacion[0] = mejorActual; // Elitismo

        for (int i = 1; i < TAM_POBLACION; i++) {
            // Torneo
            Individuo padre1 = poblacion[rand() % TAM_POBLACION];
            Individuo padre2 = poblacion[rand() % TAM_POBLACION];
            Individuo progenitor1 = esMejor(padre1.fitness, padre2.fitness) ? padre1 : padre2;

            padre1 = poblacion[rand() % TAM_POBLACION];
            padre2 = poblacion[rand() % TAM_POBLACION];
            Individuo progenitor2 = esMejor(padre1.fitness, padre2.fitness) ? padre1 : padre2;

            Individuo hijo;

            // Cruzamiento tasa de cruza
            if (((double)rand() / RAND_MAX) < TASA_CRUZA) {
                hijo.x = (rand() % 2 == 0) ? progenitor1.x : progenitor2.x;
                hijo.y = (rand() % 2 == 0) ? progenitor2.y : progenitor1.y;
            }
            else {
                hijo = (rand() % 2 == 0) ? progenitor1 : progenitor2;
            }
            
            // Mutacion inversion
            if (((double)rand() / RAND_MAX) < TASA_MUTACION) {
                hijo.x ^= (1 << (rand() % 7)); // Invierte un bit aleatorio de los 7 bits de X
                hijo.x = abs(hijo.x) % 128;
            }
            if (((double)rand() / RAND_MAX) < TASA_MUTACION) {
                hijo.y ^= (1 << (rand() % 6)); // Invierte un bit aleatorio de los 6 bits de Y
                hijo.y = abs(hijo.y) % 64;
            }

            nuevaPoblacion[i] = hijo;
        }
        poblacion = nuevaPoblacion;
    }
    cout << endl;
}

void dibujarTexto(int x, int y, string texto, void* fuente = GLUT_BITMAP_8_BY_13) {
    glRasterPos2i(x, y);
    for (char c : texto) {
        glutBitmapCharacter(fuente, c);
    }
}

void dibujarGrafica() {
    glClearColor(0.10f, 0.10f, 0.13f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int margenIzquierdo = 80;
    int margenDerecho = 720;
    int margenInferior = 80;
    int margenSuperior = 500;
    int anchoGrafica = margenDerecho - margenIzquierdo;
    int altoGrafica = margenSuperior - margenInferior;

    double maxVal = historialPromedios[0];
    for (double v : historialPromedios) if (v > maxVal) maxVal = v;
    for (double v : historialMejores) if (v > maxVal) maxVal = v;
    if (maxVal == 0) maxVal = 1;

    // Ejes
    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2d(margenIzquierdo, margenInferior); glVertex2d(margenDerecho, margenInferior);
    glVertex2d(margenIzquierdo, margenInferior); glVertex2d(margenIzquierdo, margenSuperior);
    glEnd();

    for (int i = 0; i <= GENERACIONES; i += 20) {
        int x_marca = margenIzquierdo + (i * anchoGrafica / GENERACIONES);
        glBegin(GL_LINES);
        glVertex2d(x_marca, margenInferior);
        glVertex2d(x_marca, margenInferior - 5);
        glEnd();
        string numGen = to_string(i);
        dibujarTexto(x_marca - 8, margenInferior - 20, numGen);
    }

    dibujarTexto(anchoGrafica / 2 + margenIzquierdo - 30, margenInferior - 45, "Generaciones", GLUT_BITMAP_9_BY_15);
    dibujarTexto(margenIzquierdo - 60, margenSuperior + 15, "Fitness (Adaptacion)", GLUT_BITMAP_9_BY_15);

    // Promedios (ROJO)
    glColor3f(0.9f, 0.3f, 0.3f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < GENERACIONES; i++) {
        double x_grafica = margenIzquierdo + (i * (double)anchoGrafica / (GENERACIONES - 1));
        double y_grafica = margenInferior + (historialPromedios[i] * (double)altoGrafica / maxVal);
        glVertex2d(x_grafica, y_grafica);
    }
    glEnd();

    // Mejores (VERDE)
    glColor3f(0.3f, 0.9f, 0.3f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < GENERACIONES; i++) {
        double x_grafica = margenIzquierdo + (i * (double)anchoGrafica / (GENERACIONES - 1));
        double y_grafica = margenInferior + (historialMejores[i] * (double)altoGrafica / maxVal);
        glVertex2d(x_grafica, y_grafica);
    }
    glEnd();

    int x_leyenda = 500;
    int y_leyenda = 550;

    glColor3f(0.9f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2d(x_leyenda, y_leyenda); glVertex2d(x_leyenda + 15, y_leyenda);
    glVertex2d(x_leyenda + 15, y_leyenda + 10); glVertex2d(x_leyenda, y_leyenda + 10);
    glEnd();
    glColor3f(0.9f, 0.9f, 0.9f);
    dibujarTexto(x_leyenda + 25, y_leyenda, "Promedio de Poblacion");

    glColor3f(0.3f, 0.9f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2d(x_leyenda, y_leyenda - 20); glVertex2d(x_leyenda + 15, y_leyenda - 20);
    glVertex2d(x_leyenda + 15, y_leyenda - 10); glVertex2d(x_leyenda, y_leyenda - 10);
    glEnd();
    glColor3f(0.9f, 0.9f, 0.9f);
    dibujarTexto(x_leyenda + 25, y_leyenda - 20, "Mejor Individuo (Elitismo)");

    glColor3f(1.0f, 1.0f, 1.0f);
    string titulo = "AG - MODE: " + string(OBJETIVO_ACTUAL == MINIMIZAR ? "MINIMIZACION" : "MAXIMIZACION");
    dibujarTexto(margenIzquierdo, 550, titulo, GLUT_BITMAP_HELVETICA_18);

    glutSwapBuffers();
}

void inicializarVentana() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
}

int main(int argc, char** argv) {
    srand(time(NULL));

    inicializarPoblacion();
    operarAlgoritmoGenetico();

    cout << "==================================================" << endl;
    cout << "        RESULTADOS DEL ALGORITMO GENETICO         " << endl;
    cout << "==================================================" << endl;
    cout << "Mejor Individuo encontrado en las 100 iteraciones:" << endl;
    cout << "Valor de X: " << elMejorDeTodos.x << endl;
    cout << "Valor de Y: " << elMejorDeTodos.y << endl;
    cout << "Resultado f(X,Y) [Fitness]: " << elMejorDeTodos.fitness << endl;
    cout << "==================================================" << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Algoritmo Genetico - Grafica Interactiva");

    inicializarVentana();
    glutDisplayFunc(dibujarGrafica);
    glutMainLoop();

    return 0;
}
