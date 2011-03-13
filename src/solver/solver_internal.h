#ifndef SOLVER_INTERNAL_H
#define SOLVER_INTERNAL_H

double solver_vectors_D2_Q5[][3] =
{
  { 1.0,  0.0,  0.0},
  { 0.0,  1.0,  0.0},
  {-1.0,  0.0,  0.0},
  { 0.0, -1.0,  0.0}
};

double solver_vectors_D2_Q9[][3] =
{
  { 1.0,  0.0,  0.0},
  { 0.0,  1.0,  0.0},
  {-1.0,  0.0,  0.0},
  { 0.0, -1.0,  0.0},

  { 1.0,  1.0,  0.0},
  {-1.0,  1.0,  0.0},
  {-1.0, -1.0,  0.0},
  { 1.0, -1.0,  0.0}
};

double solver_vectors_D3_Q7[][3] =
{
  { 1.0,  0.0,  0.0},
  { 0.0,  1.0,  0.0},
  {-1.0,  0.0,  0.0},
  { 0.0, -1.0,  0.0},

  { 0.0,  0.0,  1.0},
  { 0.0,  0.0, -1.0},
};

double solver_vectors_D3_Q15[][3] =
{
  { 1.0,  0.0,  0.0},
  { 0.0,  1.0,  0.0},
  {-1.0,  0.0,  0.0},
  { 0.0, -1.0,  0.0},
  { 0.0,  0.0,  1.0},
  { 0.0,  0.0, -1.0},
  
  { 1.0,  1.0,  1.0},
  { 1.0,  1.0, -1.0},
  
  {-1.0,  1.0,  1.0},
  {-1.0,  1.0, -1.0},
  
  {-1.0, -1.0,  1.0},
  {-1.0, -1.0, -1.0},
  
  { 1.0, -1.0,  1.0},
  { 1.0, -1.0, -1.0},
};

double solver_vectors_D3_Q19[][3] =
{
  { 1.0,  0.0,  0.0},
  { 0.0,  1.0,  0.0},
  {-1.0,  0.0,  0.0},
  { 0.0, -1.0,  0.0},

  { 1.0,  1.0,  0.0},
  {-1.0,  1.0,  0.0},
  {-1.0, -1.0,  0.0},
  { 1.0, -1.0,  0.0},
  
  { 0.0,  0.0,  1.0},
  { 0.0,  0.0, -1.0},
  { 0.0,  1.0,  1.0},
  { 0.0, -1.0,  1.0},
  { 0.0, -1.0, -1.0},
  { 0.0,  1.0, -1.0},
  
  { 1.0,  0.0,  1.0},
  { 1.0,  0.0, -1.0},
  {-1.0,  0.0, -1.0},
  {-1.0,  0.0,  1.0},
};

#endif /* SOLVER_INTERNAL_H */

