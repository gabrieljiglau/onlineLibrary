-- Insert genres
INSERT INTO genres (genre_id,name) VALUES (1,'Filosofie');
INSERT INTO genres (genre_id,name) VALUES (2,'Beletristica');
INSERT INTO genres (genre_id,name) VALUES (3,'Stiinte Naturale');
INSERT INTO genres (genre_id,name) VALUES (4,'Matematici Aplicate');

-- Insert subgenres
-- In Filosofie
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (1,'Metafizica', 1);
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (2,'Etica', 1);
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (3,'Politica', 1);

-- In Beletristica
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (4,'Literatura', 2);
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (5,'Poezie', 2);

-- In Stiinte Naturale
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (6,'Fizica', 3);
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (7,'Biologie', 3);
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (8,'Chimie', 3);

-- In Matematici Aplicate
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (9,'Economie', 4);
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (10,'Informatica', 4);
INSERT INTO subgenres (subgenre_id, name, genre_id) VALUES (11,'Matematica', 4);
