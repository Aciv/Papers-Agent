from pydantic import BaseModel, Field
from langchain.tools import tool
from sentence_transformers import SentenceTransformer

import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

model = SentenceTransformer(
    "./model",
    local_files_only=True  # 这个参数确保不会尝试连接网络
)
        


class PaperQuery(BaseModel):
    query: str = Field(description="查询内容，例如：'the latest research on language models'")
    last_date: str = Field(description="查询起始日期，查询在此时间之后的论文，例如：'2024-01-01'")
    author: str = Field(description="查询作者，例如：'John Doe'")
    categories: str = Field(description="arxiv类别，例如：'cs, math, eess, stat'")
    k: int =  Field(description="搜索相关论文篇数，默认为5")

import requests
@tool(args_schema=PaperQuery, 
    description="搜索学术论文的工具,传入query:论文相关信息:last_date:查询起始日期"
                "author:相关作者,k:查询论文结果个数")
def search_papers(query: str, last_date: str, author: str, categories: str, k: int ) -> str:


    logger.info(f"query : {query}\n"
                f"last_date: {last_date}\n"
                f"author: {author}\n"
                f"categories: {categories}\n")
    
    query_vector = model.encode(query)
    post_json = {
        "vector": query_vector.tolist(),
        "since_date": last_date,
        "author": author,
        "categories": categories,
        "k": k

        }
    

    url = "http://localhost:8080/admin/vector/query"

    try:
        with requests.post(url, json=post_json) as response:
            response.raise_for_status()
            # print("Status:", response.status_code)
            # print("Content-type:", response.headers['content-type'])
            response = response.text
            # print("Body:", response)
            return response
    except requests.exceptions.RequestException as e:
        print(f"请求失败: {e}")
        return f"请求失败: {e}"
    r


class PdfRead(BaseModel):
    url: str = Field(description="下载pdf的url")
import tempfile
import os
import pymupdf 

def download_pdf(url: str, save_path: str) -> bool:

    try:

        response = requests.get(url, stream=True, timeout=30)
        response.raise_for_status()  # 检查请求是否成功
        
        with open(save_path, 'wb') as f:
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:  # 过滤掉保持活动的空数据块
                    f.write(chunk)
        print(f"文件下载成功：{save_path}")
        return True
    except requests.exceptions.RequestException as e:
        print(f"下载失败：{e}")
        return False




def extract_text_with_pymupdf(pdf_path: str) -> str:
    doc = pymupdf.open(pdf_path)
    full_text = ""

    for page_num in range(len(doc)):
        page = doc.load_page(page_num)
        full_text += page.get_text()  # 获取纯文本
        
    doc.close()
    return full_text

def download_and_extract_pdf_textl(url: str) -> str:
    with tempfile.NamedTemporaryFile(delete=False, suffix=".pdf") as tmp_file:
        tmp_path = tmp_file.name

    try:
        # 1. 下载 PDF
        if not download_pdf(url, tmp_path):
            return "下载失败"
        
        # 2. 提取文字
        text = extract_text_with_pymupdf(tmp_path)
        return text
    finally:
        # 3. 清理临时文件
        if os.path.exists(tmp_path):
            os.unlink(tmp_path)

@tool(args_schema=PdfRead, description="根据url读取pdf的内容,除非用户指定查询某个论文的详细内容,否则不调用")
def pdf_read(url : str) -> str:
    return download_and_extract_pdf_textl(url)


from datetime import date
@tool(description="获取当前时间")
def getTime() -> str:
    logger.info(f"get time of {str(date.today())}")
    return str(date.today())