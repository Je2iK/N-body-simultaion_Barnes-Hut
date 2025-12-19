INSERT INTO benchmark_results (user_id, algorithm_name, particle_count, steps, duration_ms, fps_equivalent) 
VALUES ($1, $2, $3, $4, $5, $6);

SELECT * FROM benchmark_results WHERE user_id = $1 ORDER BY created_at DESC;
