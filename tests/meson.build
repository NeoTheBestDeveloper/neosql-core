criterion = libcriterion.get_variable('criterion')

e_test_header = executable('e_test_header', 'test_header.c', 
                          dependencies: criterion,
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test header.', e_test_header)

e_test_page = executable('e_test_page', 'test_page.c', 
                          dependencies: criterion,
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test page.', e_test_page)

e_test_driver = executable('e_test_driver', 'test_driver.c', 
                          dependencies: criterion,
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test driver.', e_test_driver)

e_test_connection = executable('e_test_connection', 'test_connection.c', 
                          dependencies: criterion,
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test connection.', e_test_connection)

e_test_block = executable('e_test_block', 'test_block.c', 
                          dependencies: criterion,
                          link_with: neosql_core, 
                          include_directories: incdir)
test('Test block.', e_test_block)
