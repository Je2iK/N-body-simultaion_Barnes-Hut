SELECT id, password_hash, is_admin FROM users WHERE username = $1;

INSERT INTO users (username, password_hash) VALUES ($1, $2);

DELETE FROM users WHERE id = $1;

DELETE FROM users WHERE username = $1;

UPDATE users SET is_admin = $1 WHERE id = $2;

UPDATE users SET username = $1 WHERE id = $2;

UPDATE users SET password_hash = $1 WHERE username = $2;

SELECT id, username, is_admin, created_at FROM users ORDER BY id;

SELECT id FROM users WHERE username = $1;

SELECT is_admin FROM users WHERE username = $1;

DELETE FROM users a USING users b WHERE a.id > b.id AND a.username = b.username;
