#define F_COMP(X, Y) (fabs((X) - (Y)) < 0.00001f)

typedef struct {
	float points[3];
	float vector[3];
	float force;
} EXTOBJ_force_t;

typedef struct {
	int vectors_cnt;
	int countX;
	int countY;
	int countZ;
	int obj_num;
} cl_params_t;

typedef struct {
	int forces_num;
	EXTOBJ_force_t forces[100];
} force_pack_t, *force_pack_p;

void getpos(int *counts, int node, int *pos, float *coord)
{
	int xy = counts[0] * counts[1];
	float step = 1;

	pos[2] = node / xy;
	node -= pos[2] * xy;
	pos[1] = node / counts[0];
	pos[0] = node - pos[1] * counts[0];
	if (0 != coord) {
		coord[0] = pos[0] * step;
		coord[1] = pos[1] * step;
		coord[2] = pos[2] * step;
	}
}

int nei(int *counts, int node, float *vector)
{
	float min1 = 0.577f;
	int dx = fabs(vector[0]) > min1 ? 1 : 0;
	int dy = fabs(vector[1]) > min1 ? 1 : 0;
	int dz = fabs(vector[2]) > min1 ? 1 : 0;
	int pos[3];

	getpos(counts, node, pos, 0);

	dx *= vector[0] > 0 ? 1 : -1;
	dy *= vector[1] > 0 ? 1 : -1;
	dz *= vector[2] > 0 ? 1 : -1;

	do {
		node = -1;
		if (dx < 0 && pos[0] == 0)
			break;
		if (dx > 0 && pos[0] == (counts[0] - 1))
			break;
		if (dy < 0 && pos[1] == 0)
			break;
		if (dy > 0 && pos[1] == (counts[1] - 1))
			break;
		if (dz < 0 && pos[2] == 0)
			break;
		if (dz > 0 && pos[2] == (counts[2] - 1))
			break;

		pos[0] += dx;
		pos[1] += dy;
		pos[2] += dz;

		node =
		    pos[2] * counts[0] * counts[1] + pos[1] * counts[1] +
		    pos[0];
	} while (0);

	return node;
}

float vec_mul(const float *v1, float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float feqBHK(float density, float *u, float *vector)
{
	float t = vec_mul(vector, u);
	float fnew = 1.0f + 3.0f * t + 4.5f * t * t - 1.5f * vec_mul(u, u);
	fnew *= vector[3] * density;
	return fnew;
}

__kernel void lb_bhk(__global float *us,
		     __global float *fs,
		     __global float *fsn,
		     const __global float *vectors,
		     const __global force_pack_t * forces,
		     const __global cl_params_t * params)
{
	float tau = 0.55f;
	int vectors_cnt = params->vectors_cnt;
	int counts[3] = { params->countX, params->countY, params->countZ };
	int i = get_global_id(0);
	{
		float density = 0, fe[3] = { 0, 0, 0 };
		int k, obj;
		int pos[3];
		float coord[3];

		getpos(counts, i, pos, coord);

		for (k = 0; k < vectors_cnt; ++k) {
			float f = fs[i * vectors_cnt + k];
			density += f;
			fe[0] += f * vectors[k * 4 + 0];
			fe[1] += f * vectors[k * 4 + 1];
			fe[2] += f * vectors[k * 4 + 2];
		}
		us[i * 3 + 0] = fe[0] / density;
		us[i * 3 + 1] = fe[1] / density;
		us[i * 3 + 2] = fe[2] / density;

		{
			float ux = us[i * 3 + 0];
			float uy = us[i * 3 + 1];
			float uz = us[i * 3 + 2];
			if (0.577f < sqrt(ux * ux + uy * uy + uz * uz)) {
				for (k = 0; k < vectors_cnt; ++k) {
					us[i * 3 + 0] = 0;
					us[i * 3 + 1] = 0;
					us[i * 3 + 2] = 0;
					density = 1.0f;
					fs[i * vectors_cnt + k] =
					    vectors[k * 4 + 3];
				}
			}
		}

		for (k = 0; k < vectors_cnt; ++k) {
			float nvec[4] = {
				vectors[k * 4 + 0],
				vectors[k * 4 + 1],
				vectors[k * 4 + 2],
				vectors[k * 4 + 3],
			};
			float nu[3] = {
				us[i * 3 + 0],
				us[i * 3 + 1],
				us[i * 3 + 2],
			};
			int next_node = nei(counts, i, nvec);

			if (-1 != next_node) {
				float fsi = fs[i * vectors_cnt + k];
				float feq = feqBHK(density, nu, nvec);
				float delta = (fsi - feq) / tau;
				fsn[next_node * vectors_cnt + k] += fsi - delta;
			} else {
				int opp_k;
				for (opp_k = 0; opp_k < vectors_cnt; ++opp_k) {
					if (F_COMP
					    (nvec[0], -vectors[opp_k * 4 + 0])
					    && F_COMP(nvec[1],
						      -vectors[opp_k * 4 + 1])
					    && F_COMP(nvec[2],
						      -vectors[opp_k * 4 +
							       2])) {
						break;
					}
				}

				if (opp_k < vectors_cnt) {
					fsn[i * vectors_cnt + opp_k] +=
					    fs[i * vectors_cnt + k];
				}
			}
		}

		for (obj = 0; obj < params->obj_num; ++obj) {
			int j;

			for (j = 0; j < forces[obj].forces_num; ++j) {
				float dx =
				    forces[obj].forces[j].points[0] - pos[0];
				float dy =
				    forces[obj].forces[j].points[1] - pos[1];
				float dz =
				    forces[obj].forces[j].points[2] - pos[2];
				float dist = sqrt(dx * dx + dy * dy + dz * dz);

				float d = 3.0f * 90.0f / params->countX;

				if (dist < d) {
					for (k = 0; k < vectors_cnt; ++k) {
						float nvec[3] = {
							vectors[k * 4 + 0],
							vectors[k * 4 + 1],
							vectors[k * 4 + 2]
						};
						float nforce[3] = {
							forces[obj].forces[j].
							    vector[0],
							forces[obj].forces[j].
							    vector[1],
							forces[obj].forces[j].
							    vector[2]
						};
						float delta = (d - dist) / d;
						delta *= vec_mul(nvec, nforce);
						if (fsn[i * vectors_cnt + k] +
						    delta < 0) {
							delta = 0;
						}
						fsn[i * vectors_cnt + k] +=
						    delta;
					}
				}
			}
		}
	}

}
