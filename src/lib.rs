use pyo3::prelude::*;

#[pymodule]
#[pyo3(name = "_neosql_core")]
fn neosql_core(_py: Python, m: &PyModule) -> PyResult<()> {
    Ok(())
}
