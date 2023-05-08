mat_dim = 8

import numpy as np
mat = np.random.randint(0, 100, (mat_dim, mat_dim))

# print mat to a file, using \t as delimiter
np.savetxt('matrix.txt', mat, fmt='%d', delimiter='\t')