extern crate libnewsboat;

use self::libnewsboat::configpaths::ConfigPaths;
use std::{env, path};

#[test]
fn t_configpaths_set_cache_file_changes_paths_to_cache_and_lock_files() {
    let test_dir = path::Path::new("some/dir/we/use/as/home");
    let newsboat_dir = test_dir.join(".newsboat");

    env::set_var("HOME", test_dir);

    // ConfigPaths rely on these variables, so let's sanitize them to ensure
    // that the tests aren't affected
    env::remove_var("XDG_CONFIG_HOME");
    env::remove_var("XDG_DATA_HOME");

    let mut paths = ConfigPaths::new();
    assert!(paths.initialized());

    assert_eq!(
        paths.cache_file(),
        newsboat_dir.join("cache.db").to_str().unwrap()
    );
    assert_eq!(
        paths.lock_file(),
        newsboat_dir.join("cache.db.lock").to_str().unwrap()
    );

    let new_cache = path::Path::new("something/entirely different.sqlite3");
    paths.set_cache_file(new_cache.to_string_lossy().into_owned());
    assert_eq!(paths.cache_file(), new_cache.to_str().unwrap());

    let lock_file_path = {
        let mut path = new_cache.to_string_lossy().into_owned();
        path.push_str(".lock");
        path
    };
    assert_eq!(paths.lock_file(), lock_file_path);
}
