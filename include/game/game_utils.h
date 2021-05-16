#ifndef GAME_GAME_UTILS_H_
#define GAME_GAME_UTILS_H_

static unsigned long x = 123456789, y = 362436069, z = 521288629;

unsigned long xorshf96(void);

unsigned long xorrand();


/**
 * Sine calculation using interpolated table lookup.
 * Instead of radiants or degrees we use "turns" here. Means this
 * sine does NOT return one phase for 0 to 2*PI, but for 0 to 1.
 * Input: -1 to 1 as int16 Q15  == -32768 to 32767.
 * Output: -1 to 1 as int16 Q15 == -32768 to 32767.
 *
 * @param short angle Q15
 * @return int16_t Q15
 */
short sin1(short angle);
 
/**
 * Cosine calculation using interpolated table lookup.
 * Instead of radiants or degrees we use "turns" here. Means this
 * cosine does NOT return one phase for 0 to 2*PI, but for 0 to 1.
 * Input: -1 to 1 as int16 Q15  == -32768 to 32767.
 * Output: -1 to 1 as int16 Q15 == -32768 to 32767.
 *
 * @param short angle Q15
 * @return short Q15
 */
short cos1(short angle);

#endif // end GAME_GAME_UTILS_H_