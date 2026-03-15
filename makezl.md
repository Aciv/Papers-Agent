cmake -DCMAKE_PREFIX_PATH=/root/workfolder/basic/external/Log ..

curl -s -X POST -H "Content-Type: application/json" -d @login.json http://localhost:8080/api/login | jq .

curl -b "JSESSIONID=4019f05a-61a8-4666-b8c5-16990564a2df" http://localhost:8080/api/api/user

curl -i -s -X POST -H "Content-Type: application/json" -d @login.json http://localhost:8080/api/login .

curl -s -X POST -H "Content-Type: application/json" -d @test.json http://localhost:8080/admin/vector/query | jq .