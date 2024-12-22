using AutoMapper;
using Dapper;
using fb.protocol._internal;
using Http;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("board")]
    public class BoardController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;

        public BoardController(IConfiguration configuration, IMapper mapper, DbContext dbContext)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
        }

        [HttpGet("{section}")]
        public async Task<Response.GetArticleList> GetArticleList(uint section, [FromQuery(Name = "offset")] ushort offset)
        {
            await using var conn = _dbContext.Connection(-1);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("section", section);
            dynamicParams.Add("position", offset);
            var articles = await conn.QueryAsync<Http.Model.Board>($"USP_BOARD_GET_LIST", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);

            var summaryList = _mapper.Map<List<Http.Model.Board>, List<ArticleSummary>>(articles.ToList());
            return new Response.GetArticleList
            {
                SummaryList = summaryList
            };
        }

        [HttpGet("{section}/{id}")]
        public async Task<Response.GetArticle> GetArticle(uint section, ushort id)
        {
            try
            {
                await using var conn = _dbContext.Connection(-1);
                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("section", section);
                dynamicParams.Add("article", id);
                await using var reader = await conn.QueryMultipleAsync($"USP_BOARD_GET", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);
                var article = await reader.ReadFirstOrDefaultAsync<Http.Model.Board>() ??
                    throw new LogicException(Fb.Model.EnumValue.ErrorCode.ArticleNotExists);

                var next = await reader.ReadFirstAsync<bool>();
                return new Response.GetArticle
                {
                    Article = _mapper.Map<fb.protocol._internal.Article>(article),
                    Next = next,
                    Success = true,
                };
            }
            catch (Exception e)
            {
                return new Response.GetArticle
                {
                    Success = false
                };
            }
        }

        [HttpPost("write")]
        public async Task<Response.WriteArticle> Write(Request.WriteArticle request)
        {
            await using var conn = _dbContext.Connection(-1);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("section", request.Section);
            dynamicParams.Add("uid", request.User);
            dynamicParams.Add("title", request.Title);
            dynamicParams.Add("contents", request.Contents);
            var result = await conn.ExecuteAsync($"USP_BOARD_ADD", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);

            return new Response.WriteArticle
            {
                Success = result == 1
            };
        }

        [HttpPost("delete")]
        public async Task<Response.DeleteArticle> Delete(Request.DeleteArticle request)
        {
            await using var conn = _dbContext.Connection(-1);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("id", request.Id);
            dynamicParams.Add("user", request.User);
            var result = await conn.ExecuteScalarAsync<int>($"USP_BOARD_DELETE", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);

            return new Response.DeleteArticle
            {
                Result = result
            };
        }
    }
}