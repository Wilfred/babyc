/* BabyC a toy compiler */

int main() {
    int x, y, z;

    x = -2;

    while (x < 3) {
        y = -2;

        while (y < 3) {
            z = -2;

            while (z < 3) {

                int w =
                    ((((x * x - (!y ^ !!y)) +
                       ((x + (((x + (z * z - 0x1)) *
                               (x + ((z + 0x1) * (z - 0x1)))) *
                                  ((x + (z * z - 0x1)) *
                                   (x + ((z + 0x1) * (z - 0x1)))) -
                              (!(z || (z && y)) ^ !!z))) *
                        (x +
                         (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                            0x2 * x * ((z + 0x1) * (z - 0x1))) +
                           (!(!z || !y) ^ !!!(z && y))) *
                          (((x + (z * z - 0x1)) *
                            (x + ((z + 0x1) * (z - 0x1)))) -
                           (!(z || (z && y)) ^ !!z)))))) *
                      (((x + (!!((x || (z && x)) >= !(z >= y)) ^
                              !!(((x || z) && (x || x)) < (z < y)))) *
                        (x - (!y ^ !!y))) +
                       (x * x +
                        (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                           0x2 * x * ((z + 0x1) * (z - 0x1))) +
                          (!(!z || !y) ^ !!!(z && y))) *
                         (((x + (z * z - 0x1)) *
                           (x + ((z + 0x1) * (z - 0x1)))) -
                          (!(z || (z && y)) ^ !!z))) *
                            (((x + (z * z - 0x1)) *
                              (x + ((z + 0x1) * (z - 0x1)))) *
                                 ((x + (z * z - 0x1)) *
                                  (x + ((z + 0x1) * (z - 0x1)))) -
                             (!(z || (z && y)) ^ !!z)) +
                        0x2 * x *
                            (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                               0x2 * x * ((z + 0x1) * (z - 0x1))) +
                              (!(!z || !y) ^ !!!(z && y))) *
                             (((x + (z * z - 0x1)) *
                               (x + ((z + 0x1) * (z - 0x1)))) -
                              (!(z || (z && y)) ^ !!z)))))) *
                         (((x * x - (!y ^ !!y)) +
                           ((x + (((x + (z * z - 0x1)) *
                                   (x + ((z + 0x1) * (z - 0x1)))) *
                                      ((x + (z * z - 0x1)) *
                                       (x + ((z + 0x1) * (z - 0x1)))) -
                                  (!(z || (z && y)) ^ !!z))) *
                            (x + (((x * x +
                                    ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                                    0x2 * x * ((z + 0x1) * (z - 0x1))) +
                                   (!(!z || !y) ^ !!!(z && y))) *
                                  (((x + (z * z - 0x1)) *
                                    (x + ((z + 0x1) * (z - 0x1)))) -
                                   (!(z || (z && y)) ^ !!z)))))) *
                          (((x + (!!((x || (z && x)) >= !(z >= y)) ^
                                  !!(((x || z) && (x || x)) < (z < y)))) *
                            (x - (!y ^ !!y))) +
                           (x * x +
                            (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                               0x2 * x * ((z + 0x1) * (z - 0x1))) +
                              (!(!z || !y) ^ !!!(z && y))) *
                             (((x + (z * z - 0x1)) *
                               (x + ((z + 0x1) * (z - 0x1)))) -
                              (!(z || (z && y)) ^ !!z))) *
                                (((x + (z * z - 0x1)) *
                                  (x + ((z + 0x1) * (z - 0x1)))) *
                                     ((x + (z * z - 0x1)) *
                                      (x + ((z + 0x1) * (z - 0x1)))) -
                                 (!(z || (z && y)) ^ !!z)) +
                            0x2 * x *
                                (((x * x +
                                   ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                                   0x2 * x * ((z + 0x1) * (z - 0x1))) +
                                  (!(!z || !y) ^ !!!(z && y))) *
                                 (((x + (z * z - 0x1)) *
                                   (x + ((z + 0x1) * (z - 0x1)))) -
                                  (!(z || (z && y)) ^ !!z)))))) -
                     (!!((!((z || (z && y)) && (z || (y || (z && x)))) &&
                          !!((z || (z && x)) > (z && (!z || y)))) >=
                         (z ||
                          ((y || (y && (z || (z && y)))) &&
                           ((x || (x && z)) && (x || (y && (!y || x))))))) ^
                      !!(!(z || (((z || z) && (z || x)) <= (z && y))) <
                         ((z || y) && (z || x))))) ==
                    (((((x + (!!((x || (z && x)) >= !(z >= y)) ^
                              !!(((x || z) && (x || x)) < (z < y)))) *
                        (x - (!y ^ !!y))) *
                           (x * x - (!y ^ !!y)) +
                       (x * x +
                        (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                           0x2 * x * ((z + 0x1) * (z - 0x1))) +
                          (!(!z || !y) ^ !!!(z && y))) *
                         (((x + (z * z - 0x1)) *
                           (x + ((z + 0x1) * (z - 0x1)))) -
                          (!(z || (z && y)) ^ !!z))) *
                            (((x + (z * z - 0x1)) *
                              (x + ((z + 0x1) * (z - 0x1)))) *
                                 ((x + (z * z - 0x1)) *
                                  (x + ((z + 0x1) * (z - 0x1)))) -
                             (!(z || (z && y)) ^ !!z)) +
                        0x2 * x *
                            (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                               0x2 * x * ((z + 0x1) * (z - 0x1))) +
                              (!(!z || !y) ^ !!!(z && y))) *
                             (((x + (z * z - 0x1)) *
                               (x + ((z + 0x1) * (z - 0x1)))) -
                              (!(z || (z && y)) ^ !!z)))) *
                           ((x + (((x + (z * z - 0x1)) *
                                   (x + ((z + 0x1) * (z - 0x1)))) *
                                      ((x + (z * z - 0x1)) *
                                       (x + ((z + 0x1) * (z - 0x1)))) -
                                  (!(z || (z && y)) ^ !!z))) *
                            (x + (((x * x +
                                    ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                                    0x2 * x * ((z + 0x1) * (z - 0x1))) +
                                   (!(!z || !y) ^ !!!(z && y))) *
                                  (((x + (z * z - 0x1)) *
                                    (x + ((z + 0x1) * (z - 0x1)))) -
                                   (!(z || (z && y)) ^ !!z))))) +
                       0x2 * (x * x - (!y ^ !!y)) *
                           (x * x +
                            (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                               0x2 * x * ((z + 0x1) * (z - 0x1))) +
                              (!(!z || !y) ^ !!!(z && y))) *
                             (((x + (z * z - 0x1)) *
                               (x + ((z + 0x1) * (z - 0x1)))) -
                              (!(z || (z && y)) ^ !!z))) *
                                (((x + (z * z - 0x1)) *
                                  (x + ((z + 0x1) * (z - 0x1)))) *
                                     ((x + (z * z - 0x1)) *
                                      (x + ((z + 0x1) * (z - 0x1)))) -
                                 (!(z || (z && y)) ^ !!z)) +
                            0x2 * x *
                                (((x * x +
                                   ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                                   0x2 * x * ((z + 0x1) * (z - 0x1))) +
                                  (!(!z || !y) ^ !!!(z && y))) *
                                 (((x + (z * z - 0x1)) *
                                   (x + ((z + 0x1) * (z - 0x1)))) -
                                  (!(z || (z && y)) ^ !!z))))) +
                      (!!(!(!!(z > x) > z) >
                          (!(y && (y || (x && (!x || z)))) &&
                           !((x || (x && z)) ||
                             (((x || x) && (x || z)) && !(z > x))))) ^
                       !!(((1 - !!(z <= x)) <= z) <=
                          !(y || ((x || x) && (x || z)))))) *
                     ((((x * x - (!y ^ !!y)) +
                        ((x + (((x + (z * z - 0x1)) *
                                (x + ((z + 0x1) * (z - 0x1)))) *
                                   ((x + (z * z - 0x1)) *
                                    (x + ((z + 0x1) * (z - 0x1)))) -
                               (!(z || (z && y)) ^ !!z))) *
                         (x +
                          (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                             0x2 * x * ((z + 0x1) * (z - 0x1))) +
                            (!(!z || !y) ^ !!!(z && y))) *
                           (((x + (z * z - 0x1)) *
                             (x + ((z + 0x1) * (z - 0x1)))) -
                            (!(z || (z && y)) ^ !!z)))))) *
                       (((x + (!!((x || (z && x)) >= !(z >= y)) ^
                               !!(((x || z) && (x || x)) < (z < y)))) *
                         (x - (!y ^ !!y))) +
                        (x * x +
                         (((x * x + ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                            0x2 * x * ((z + 0x1) * (z - 0x1))) +
                           (!(!z || !y) ^ !!!(z && y))) *
                          (((x + (z * z - 0x1)) *
                            (x + ((z + 0x1) * (z - 0x1)))) -
                           (!(z || (z && y)) ^ !!z))) *
                             (((x + (z * z - 0x1)) *
                               (x + ((z + 0x1) * (z - 0x1)))) *
                                  ((x + (z * z - 0x1)) *
                                   (x + ((z + 0x1) * (z - 0x1)))) -
                              (!(z || (z && y)) ^ !!z)) +
                         0x2 * x * (((x * x +
                                      ((z + 0x1) * (z - 0x1)) * (z * z - 0x1) +
                                      0x2 * x * ((z + 0x1) * (z - 0x1))) +
                                     (!(!z || !y) ^ !!!(z && y))) *
                                    (((x + (z * z - 0x1)) *
                                      (x + ((z + 0x1) * (z - 0x1)))) -
                                     (!(z || (z && y)) ^ !!z)))))) -
                      (!!((!((z || (z && y)) && (z || (y || (z && x)))) &&
                           !!((z || (z && x)) > (z && (!z || y)))) >=
                          (z ||
                           ((y || (y && (z || (z && y)))) &&
                            ((x || (x && z)) && (x || (y && (!y || x))))))) ^
                       !!(!(z || (((z || z) && (z || x)) <= (z && y))) <
                          ((z || y) && (z || x))))));
                if (w != 1)
                    goto err;
                z = z + 1;
            }
            y = y + 1;
        }
        x = x + 1;
    }
    return 0;
err:
    return 1;
}
