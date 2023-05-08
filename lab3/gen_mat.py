mat_dim = 960*10

import numpy as np
mat = np.random.randint(0, 100, (mat_dim, mat_dim))

# print mat to a file, using \t as delimiter
np.savetxt(f'matrix_{mat_dim}.txt', mat, fmt='%d', delimiter='\t')
