use regex::Regex;
use std::error::Error;
use std::{env, fs};

#[derive(Debug)]
pub struct Config {
    pub query: String,
    pub filename: String,
    pub case_sensitive: bool,
}

impl Config {
    pub fn new(mut args: std::env::Args) -> Result<Config, &'static str> {
        // args is an iterator
        args.next();

        let query = match args.next() {
            Some(arg) => arg,
            None => return Err("Didn't get a query string"),
        };

        let filename = match args.next() {
            Some(arg) => arg,
            None => return Err("Didn't get a file name"),
        };

        return Ok(Config {
            query,
            filename,
            case_sensitive: env::var("CASE_INSENSITIVE").is_err(),
        });
    }
}

pub fn run(config: Config) -> Result<(), Box<dyn Error>> {
    let res = fs::read_to_string(&config.filename);
    match res {
        Ok(contents) => {
            if config.case_sensitive {
                match search(&config.query, &contents) {
                    Ok(matched_lines) => {
                        print_result(&matched_lines);
                        Ok(())
                    }
                    Err(e) => Err(Box::from(e)),
                }
            } else {
                match search_case_insensitive(&config.query, &contents) {
                    Ok(matched_lines) => {
                        print_result(&matched_lines);
                        Ok(())
                    }
                    Err(e) => Err(Box::from(e)),
                }
            }
        }
        Err(e) => Err(Box::from(e)),
    }
}

fn search<'a>(query: &str, contents: &'a str) -> Result<Vec<&'a str>, Box<dyn Error>> {
    // return lifetime must within contents
    let regex = Regex::new(query)?;
    return Ok(contents
        .lines()
        .filter(|line| regex.find(line).is_some())
        .collect());
}

fn print_result(matched_lines: &Vec<&str>) {
    for matched_line in matched_lines {
        println!("{}", matched_line);
    }
}

fn search_case_insensitive<'a>(
    query: &str,
    contents: &'a str,
) -> Result<Vec<&'a str>, Box<dyn Error>> {
    let regex = Regex::new(query.to_lowercase().as_str())?;
    return Ok(contents
        .lines()
        .filter(|line| regex.find(line.to_lowercase().as_str()).is_some())
        .collect());
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn case_regex() {
        let query = "^R";
        let contents = "\
Rust:
safe, fast, productive.
Pick three.";
        assert_eq!(vec!["Rust:"], search(query, contents).unwrap())
    }

    #[test]
    fn case_sensitive() {
        let query = "Rust";
        let contents = "\
Rust:
safe, fast, productive.
Pick three.";
        assert_eq!(vec!["Rust:"], search(query, contents).unwrap())
    }

    #[test]
    fn case_insensitive() {
        let query = "rUsT";
        let contents = "\
Rust:
safe, fast, productive.
Pick three.";
        assert_eq!(
            vec!["Rust:"],
            search_case_insensitive(query, contents).unwrap()
        )
    }
}
