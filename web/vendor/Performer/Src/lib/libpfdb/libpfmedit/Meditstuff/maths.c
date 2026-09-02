/*--------------------------Message start-------------------------------

This software is copyright (C) 1994-1995 by Medit Productions


This software is provided under the following terms, if you do not
agree to all of these terms, delete this software now. Any use of
this software indicates the acceptance of these terms by the person
making said usage ("The User").

1) Medit Productions provides this software "as is" and without
warranty of any kind. All consequences of the use of this software
fall completely on The User.

2) This software may be copied without restriction, provided that an
unaltered and clearly visible copy of this message is placed in any
copies of, or derivatives of, this software.


---------------------------Message end--------------------------------*/




/************************************************************************
Maths routines  nb. None of these are fast or optimised
************************************************************************/
Matrix IdentityMatrix = { { 1.0, 0.0, 0.0, 0.0},
			  { 0.0, 1.0, 0.0, 0.0},
			  { 0.0, 0.0, 1.0, 0.0},
			  { 0.0, 0.0, 0.0, 1.0} };
#define Identify(m) CopyMatrix(m, IdentityMatrix)
#define Identifyf(m) RealMatrixToFloat(IdentityMatrix, m)

#define CP(x,y) Dest[x][y] = Source[x][y]
void CopyMatrix(reg MatrixPtr Dest, reg MatrixPtr Source)
{
    CP(X,X); CP(X,Y); CP(X,Z); CP(X,W);
    CP(Y,X); CP(Y,Y); CP(Y,Z); CP(Y,W);
    CP(Z,X); CP(Z,Y); CP(Z,Z); CP(Z,W);
    CP(W,X); CP(W,Y); CP(W,Z); CP(W,W);
}
void RealMatrixToFloat(reg MatrixPtr Source, reg float (*Dest)[4])
{
	CP(X,X); CP(X,Y); CP(X,Z); CP(X,W);
	CP(Y,X); CP(Y,Y); CP(Y,Z); CP(Y,W);
	CP(Z,X); CP(Z,Y); CP(Z,Z); CP(Z,W);
	CP(W,X); CP(W,Y); CP(W,Z); CP(W,W);
}
#undef CP

#define MM(x) (a[i][x]*b[x][j])
void MultMatrix(reg MatrixPtr a, reg MatrixPtr b, reg MatrixPtr Result)
{
    Matrix temp;
    register int i,j;

    for (i=0; i<XYZW; i++) {
	for (j=0; j<XYZW; j++) {
	    temp[i][j] = MM(X) + MM(Y) + MM(Z) + MM(W);
	}
    }
    CopyMatrix(Result, temp);
}
#undef MM

/************************************************************************
Coordinate transformations
************************************************************************/
#define TF(c) (px*m[X][c]) + (py*m[Y][c]) + (pz*m[Z][c]) + m[W][c]
void TransformCoordinate(CoordinatePtr p, MatrixPtr m, CoordinatePtr Result)
{
    register real x,y,z;
    register real px = *p++,  py = *p++,  pz = *p;

    x = TF(X);		y = TF(Y);	    z = TF(Z);
    *Result++ = x;	*Result++ = y;	    *Result = z;
}
void TransformCoordinatef(float *p, MatrixPtr m, float *Result)
{
    register real x,y,z;
    register real px = *p++,  py = *p++,  pz = *p;

    x = TF(X);		y = TF(Y);	    z = TF(Z);
    *Result++ = x;	*Result++ = y;	    *Result = z;
}
#undef TF
#define RT(c) (vx*m[X][c]) + (vy*m[Y][c]) + (vz*m[Z][c])
void RotateVector(VectorPtr v, MatrixPtr m, VectorPtr Result)
{
    register real x,y,z;
    register real vx = v[X],  vy = v[Y],  vz = v[Z];

    x = RT(X);		y = RT(Y);	    z = RT(Z);
    Result[X] = x;	Result[Y] = y;	    Result[Z] = z;
}
#undef RT

/************************************************************************
Matrix inverse
************************************************************************/
#define COF(x,y) m[ilist[x]][jlist[y]]
#define DET(x) (m[0][x]*Cofactor[0][x])
MatrixPtr MatrixInverse(reg MatrixPtr m)
{
    static Matrix inv;
    int i, j, k;
    Matrix Cofactor;
    real Determinant;
    int iindex, ilist[XYZW-1], jindex, jlist[XYZW-1];

    for (i=0; i<XYZW; i++) {
	for (j=0; j<XYZW; j++) {
	    iindex = jindex = 0;
	    for (k=0; k<XYZW; k++) {
		if (k ISNT i) ilist[iindex++] = k;
		if (k ISNT j) jlist[jindex++] = k;
	    }
	    /* Do 3x3 determinant for each cofactor */
	    Cofactor[i][j] = ( (COF(0,0)*COF(1,1)*COF(2,2)) +
			       (COF(0,1)*COF(1,2)*COF(2,0)) +
			       (COF(0,2)*COF(1,0)*COF(2,1)) ) -
			     ( (COF(0,0)*COF(1,2)*COF(2,1)) +
			       (COF(0,1)*COF(1,0)*COF(2,2)) +
			       (COF(0,2)*COF(1,1)*COF(2,0)) );
	    /* Work out place sign... */
	    if ((i + j) & 1) Cofactor[i][j] = -Cofactor[i][j];
	}
    }
    Determinant = DET(0) + DET(1) + DET(2) + DET(3);
    if (Determinant IS 0.0) {
	fprintf(stderr, "Failed to invert matrix because determinant=0\n");
	return IdentityMatrix;
    }
    for (i=0; i<XYZW; i++) {
	for (j=0; j<XYZW; j++) {
	    inv[i][j] = Cofactor[j][i] / Determinant;
	}
    }
    return inv;
}
#define InvertMatrix(source,dest) CopyMatrix(dest,MatrixInverse(source))
/************************************************************************
Modelling transformations
************************************************************************/
MatrixPtr YawMatrix(real Degrees)
{
    static Matrix m;
    register real sina = sin(rad(Degrees));
    register real cosa = cos(rad(Degrees));
    register real zero = 0.0, one = 1.0;
    m[0][0] =  cosa;   m[0][1] =  sina;   m[0][2] =  zero;   m[0][3] =  zero;
    m[1][0] = -sina;   m[1][1] =  cosa;   m[1][2] =  zero;   m[1][3] =  zero;
    m[2][0] =  zero;   m[2][1] =  zero;   m[2][2] =   one;   m[2][3] =  zero;
    m[3][0] =  zero;   m[3][1] =  zero;   m[3][2] =  zero;   m[3][3] =   one;
    return m;
}
MatrixPtr PitchMatrix(real Degrees)
{
    static Matrix m;
    register real sina = sin(rad(Degrees));
    register real cosa = cos(rad(Degrees));
    register real zero = 0.0, one = 1.0;
    m[0][0] =   one;   m[0][1] =  zero;   m[0][2] =  zero;   m[0][3] =  zero;
    m[1][0] =  zero;   m[1][1] =  cosa;   m[1][2] =  sina;   m[1][3] =  zero;
    m[2][0] =  zero;   m[2][1] = -sina;   m[2][2] =  cosa;   m[2][3] =  zero;
    m[3][0] =  zero;   m[3][1] =  zero;   m[3][2] =  zero;   m[3][3] =   one;
    return m;
}
MatrixPtr RollMatrix(real Degrees)
{
    static Matrix m;
    register real sina = sin(rad(Degrees));
    register real cosa = cos(rad(Degrees));
    register real zero = 0.0, one = 1.0;
    m[0][0] =  cosa;   m[0][1] =  zero;   m[0][2] = -sina;   m[0][3] =  zero;
    m[1][0] =  zero;   m[1][1] =   one;   m[1][2] =  zero;   m[1][3] =  zero;
    m[2][0] =  sina;   m[2][1] =  zero;   m[2][2] =  cosa;   m[2][3] =  zero;
    m[3][0] =  zero;   m[3][1] =  zero;   m[3][2] =  zero;   m[3][3] =   one;
    return m;
}
MatrixPtr TranslationMatrix(real x, real y, real z)
{
    static Matrix m;
    register real zero = 0.0, one = 1.0;
    m[0][0] =  one; m[0][1] = zero; m[0][2] = zero; m[0][3] = zero;
    m[1][0] = zero; m[1][1] =  one; m[1][2] = zero; m[1][3] = zero;
    m[2][0] = zero; m[2][1] = zero; m[2][2] =  one; m[2][3] = zero;
    m[3][0] =    x; m[3][1] =    y; m[3][2] =    z; m[3][3] =  one;
    return m;
}
MatrixPtr ScaleMatrix(real x, real y, real z)
{
    register MatrixPtr m;
    register real zero = 0.0, one = 1.0;

    static Matrix Result1,Result2;
    static flag WhichOne = FALSE;
    m = (WhichOne = !WhichOne)?Result1:Result2;
    
    m[0][0] =    x; m[0][1] = zero; m[0][2] = zero; m[0][3] = zero;
    m[1][0] = zero; m[1][1] =    y; m[1][2] = zero; m[1][3] = zero;
    m[2][0] = zero; m[2][1] = zero; m[2][2] =    z; m[2][3] = zero;
    m[3][0] = zero; m[3][1] = zero; m[3][2] = zero; m[3][3] =  one;
    return m;
}

/* Premultiplied modelling transformations */
#define PreYaw(matrix,degrees)     MultMatrix(YawMatrix(degrees),      matrix,matrix)
#define PrePitch(matrix,degrees)   MultMatrix(PitchMatrix(degrees),    matrix,matrix)
#define PreRoll(matrix,degrees)    MultMatrix(RollMatrix(degrees),     matrix,matrix)
#define PreTranslate(matrix,x,y,z) MultMatrix(TranslationMatrix(x,y,z),matrix,matrix)
#define PreScale(matrix,x,y,z)     MultMatrix(ScaleMatrix(x,y,z),      matrix,matrix)

/* Postmultiplied versions of the above... */
#define PostYaw(matrix,degrees)     MultMatrix(matrix,YawMatrix(degrees),      matrix)
#define PostPitch(matrix,degrees)   MultMatrix(matrix,PitchMatrix(degrees),    matrix)
#define PostRoll(matrix,degrees)    MultMatrix(matrix,RollMatrix(degrees),     matrix)
#define PostTranslate(matrix,x,y,z) MultMatrix(matrix,TranslationMatrix(x,y,z),matrix)
#define PostScale(matrix,x,y,z)     MultMatrix(matrix,ScaleMatrix(x,y,z),      matrix)

/************************************************************************
Generate a matrix to rotate a vector to the X, Y or Z axis (whichever
is nearest), return that axis
************************************************************************/
int RotateToAxis(VectorPtr v, MatrixPtr m)
{
    Vector v1;
    register real angle, x = fabs(v[X]), y = fabs(v[Y]), z = fabs(v[Z]);

    if ((x > y) AND (x > z)) {		/* Go to X axis */
	angle = deg(atan2(v[Z],v[X]));
	CopyMatrix(m,RollMatrix(angle));
	RotateVector(v,m,v1);
	angle = deg(atan2(v1[Y],v1[X]));
	PostYaw(m,-angle);
	return X;
    }
    else if (y > z) {			/* Go to Y axis */
	angle = deg(atan2(v[X],v[Y]));
	CopyMatrix(m,YawMatrix(angle));
	RotateVector(v,m,v1);
	angle = deg(atan2(v1[Z],v1[Y]));
	PostPitch(m,-angle);
	return Y;
    }
    else {				/* Go to Z axis */
	angle = deg(atan2(v[Y],v[Z]));
	CopyMatrix(m,PitchMatrix(angle));
	RotateVector(v,m,v1);
	angle = deg(atan2(v1[X],v1[Z]));
	PostRoll(m,-angle);
	return Z;
    }
}
void RotateToXAxis(VectorPtr v, MatrixPtr m)
{
    switch (RotateToAxis(v, m)) {
	case X:	if (v[X] < 0) {
		    PostPitch(m, 180.0);
		}
		break;
	case Y:	PostYaw(m, -90.0);
		break;
	case Z:	PostRoll(m, 90.0);
		/*if (v[Z] > 0)*/ {
		    PostPitch(m, 90.0);
		}
		break;
    }
}
/************************************************************************
Calculate a polygon normal
************************************************************************/
static real LengthOfVector(reg VectorPtr v)
{
    return sqrt(square(v[X])+square(v[Y])+square(v[Z]));
}
static boolean CalcPolyNormal(PolygonPtr p, VectorPtr v)
{
    reg int i, n;
    reg real xa,ya,za,xb,yb,zb,length;

    n = 1;
    v[X] = v[Y] = v[Z] = 0.0;
    xa = p->Pos[0][X]; ya = p->Pos[0][Y]; za = p->Pos[0][Z];
    for (i=0; i<p->NoVerts; i++) {
	xb = p->Pos[n][X]; yb = p->Pos[n][Y]; zb = p->Pos[n][Z];

	v[X] += (ya-yb)*(za+zb);
	v[Y] += (za-zb)*(xa+xb);
	v[Z] += (xa-xb)*(ya+yb);

	xa = xb; ya = yb; za = zb;
	if (++n IS p->NoVerts) {
	    n = 0;
	}
    }
    length = LengthOfVector(v);
    if (length ISNT 0.0) {
	v[X] /= length;
	v[Y] /= length;
	v[Z] /= length;
	return TRUE;
    }
    else {
	return FALSE;
    }
}
