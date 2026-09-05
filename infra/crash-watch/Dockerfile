FROM python:3.12-slim

WORKDIR /app
RUN pip install --no-cache-dir kubernetes==32.0.1 docker==7.1.0
COPY watch.py /app/watch.py

ENV PYTHONUNBUFFERED=1
CMD ["python", "/app/watch.py"]
