#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Nach http://student.ulb.ac.be/~claugero/sphere/index.html
# Optimierungen wurden weggelassen.

from math import *
import sys
import pprint

t = (1.0 + sqrt(5.0)) / 2.0;
s = sqrt(1 + t ** 2)
a = t / s;
b = 1.0 / s;

# 5 ist der Suedpol

vertices = [[a, b, 0.0], [-a, b, 0.0],[a, -b, 0.0],
            [-a, -b, 0.0],[b, 0.0 , a],[b, 0.0 , -a],
            [-b, 0.0 , a],[-b, 0.0 , -a],[0.0 , a, b],
            [0.0 , -a, b],[0.0 , a, -b],[0.0 , -a, -b]];

triangles = [
  [0,8,4], [1,10,7], [2,9,11], [7,3,1],
  [0,5,10],[3,9,6], [3,11,9], [8,6,4],
  [2,4,9], [3,7,11],[4,2,0], [9,4,6],
  [2,11,5],[0,10,8],[5,0,2], [10,5,7],
  [1,6,8], [1,8,10],[6,1,3], [11,7,5]
];

#triangles = [
#	[0,5,10],
#	[2,11,5],
#	[5,0,2],
#	[10,5,7], #Suedpol 1
#	[11,7,5], #Suedpol 2
#];

# Die Mitte von 5 und 7 ist der Suedpol


class Triangle:
  def __init__(self, p1, p2, p3, parent = None):
    '''Diese Methode initialisiert das Dreieck'''
    self.vertices = [p1,p2,p3]
    self.texcoords = self.calculateTexCords(self.vertices)
    self.parent = parent
    self.children = []

  def calculateTexCords(self, vertices):
    texcoords = []
    coords = None
    for v in range(len(vertices)):
      '''Wenn die ersten beiden Kooridination 0 sind, dann sind wir an einem der Pole. Ohne diesen Fix gibt es nur eine Texturkoordinate fuer die Pole. Damit landet die X-Koordinate bei 0.5. Damit die Texture korrekt an der Kante entlang geht wird der Pole etwas in Richtung der 2 anderen zum Dreieck gehoerenden Vertives verschoben. Damit wird eine bessere Verteilung auf die x-Texturkoordinate erreicht.'''
      if (vertices[v][0] == 0 and vertices[v][1] == 0):
        if v == 0:
          fix = self.middlePoint(vertices[1], vertices[2])
        elif v == 1:
          fix = self.middlePoint(vertices[0], vertices[2])
        else: # v == 2
          fix = self.middlePoint(vertices[0], vertices[1])
        vertices[v] = [fix[0] / 10000000, fix[1] / 10000000, vertices[v][2]]

      coords = self.texCoords(self.sphericalToGeographic(self.cartesianToSpherical(vertices[v])), coords)
      texcoords.append(coords)
    return texcoords

  def save(self, vert_out):
     '''Diese Methoe speichert die Daten. Falls die aktuelle Klasse Kinder hat ist dies rekursiv.'''
     if (len(self.children) == 0):
       for v in range(len(self.vertices)):
           vert_out.writelines(str(tp) + ', ' for tp in self.texcoords[v])
           vert_out.write('\n')
           vert_out.writelines(str(p) + ', ' for p in self.vertices[v])
           vert_out.write('\n')
     else:
       for c in self.children:
          c.save(vert_out)

  def getTriangles(self):
     if (len(self.children) == 0):
       return [self]
     else:
       res = []
       for c in self.children:
          t = c.getTriangles()
          for i in t:
            res.append(i)
       return res

  def middlePoint(self, a, b):
    '''Diese Methode berechnet den Mittelpunkt zweier Ortsvektoren'''
    return [
      (a[0] + b[0]) / 2.0,
      (a[1] + b[1]) / 2.0,
      (a[2] + b[2]) / 2.0,
    ]

  def vectorLength(self, v):
    '''Diese Methode berechnet die Länge eines Vektors'''
    res = 0
    for p in v:
      res += p ** 2
    return sqrt(res)

  def cartesianToSpherical(self, v):
    '''Diese Methode berechnet zu einen gegebenen Ortsvektor aus R3 die sphaerischen Koordination'''
    rho = self.vectorLength(v)
    phi = atan2(v[1], v[0]) + pi # Addiere pi damit phi zwischen 0 und 2 pi landet
    if (v[0] != 0 or v[1] != 0):
      theta = pi / 2 - atan(v[2] / self.vectorLength([v[0], v[1]]))
    else:
      theta = 0
    return (rho, phi, theta)

  def sphericalToGeographic(self, v):
    '''Diese Methode entfernt den Radius und wandelt die spherischen koordinaten somit in geographische um'''
    return (v[1], v[2])

  def texCoords(self,v, last = None):
    '''Diese Methode wandelt die spherischen Kooridinaten in Texturkoordinaten um. y Invertiert da Windows Bitmaps auf dem Kopf stehen'''
    x = v[0] / (2 * pi) 
    y = 1- v[1] / (pi)

    # Korrektur des Fehlers bei überlappenden Texturkoordinaten
    if (last != None and last[0] > 0.75 and x < 0.25):
       x = x + 1
    if (last != None and last[0] < 0.25 and x > 0.75):
       x = x - 1

    return (x,y)

  def normalize(self, a):
    '''Diese Methode normalisiert den Vektor auf 1'''
    length = self.vectorLength(a);
    if (length > 0):
      length = 1.0 / length
    return [v * length for v in a]

  def split(self, maxdepth, depth = 1):
    '''Diese Methode teilt das Dreieck in 4 neue Dreiecke'''
    if maxdepth >= depth:
      points = [
        self.normalize(self.middlePoint(self.vertices[0], self.vertices[1])),
        self.normalize(self.middlePoint(self.vertices[1], self.vertices[2])),
        self.normalize(self.middlePoint(self.vertices[0], self.vertices[2]))
      ]

      self.children = [
        Triangle(self.vertices[0], points[0], points[2], self),
        Triangle(points[0], self.vertices[1], points[1], self),
        Triangle(points[2], points[1], self.vertices[2], self),
        Triangle(points[1], points[2], points[0], self)
      ]
      if maxdepth > depth:
        for c in self.children:
          c.split(maxdepth, depth + 1)

if __name__ == '__main__':
  out = open('Globe.h', 'w')
  out.write('static double GlobeNodes[] = {\n')

  splits = 4

  for f in triangles:
    tri = Triangle(vertices[f[0]], vertices[f[1]], vertices[f[2]])
    tri.split(splits)
    tri.save(out)
  out.write('};\n')
  out.write('static unsigned int GlobeCount = ' + str(3*20*4**splits) + ';\n\n')
  out.close()
