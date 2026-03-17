cmake -DCMAKE_PREFIX_PATH=/root/workfolder/basic/external/Log ..

curl -s -X POST -H "Content-Type: application/json" -d @login.json http://localhost:8080/api/login | jq .

curl -b "JSESSIONID=694b18ca-eb71-4b01-882d-93f5c114cd42" http://localhost:8080/api/user

curl -i -s -X POST -H "Content-Type: application/json" -d @login.json http://localhost:8080/api/login 

curl -s -X POST -H "Content-Type: application/json" -d @test.json http://localhost:8080/admin/vector/query | jq .


curl -s -X POST -H "Content-Type: application/json" -d @add.json -b "JSESSIONID=fba9626f-78a0-4dde-b3b0-6970cf65603b" http://localhost:8080/api/paper/add

select u.arxiv_id, p.title from user_collections u 
                          inner join papers p on u.arxiv_id = p.arxiv_id 
                          where u.user_id = 2;