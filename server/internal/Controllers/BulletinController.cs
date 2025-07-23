using AutoMapper;
using Dapper;
using fb.protocol._internal;
using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    /// <summary>
    /// Provides bulletin operations for the internal API.
    /// Handles article retrieval, writing, and deletion for the in-game bulletin system.
    /// </summary>
    [ApiController]
    [Route("bulletin")]
    public class BulletinController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;

        /// <summary>
        /// Initializes a new instance of the <see cref="BulletinController"/> class.
        /// </summary>
        /// <param name="configuration">The application configuration.</param>
        /// <param name="mapper">The AutoMapper instance for object mapping.</param>
        /// <param name="dbContext">The database context for data operations.</param>
        public BulletinController(IConfiguration configuration, IMapper mapper, DbContext dbContext)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
        }

        /// <summary>
        /// Retrieves a paginated list of articles from a specific bulletin section.
        /// Returns article summaries using a stored procedure for efficient pagination.
        /// </summary>
        /// <param name="section">The bulletin section ID to retrieve articles from.</param>
        /// <param name="offset">The starting position for pagination.</param>
        /// <returns>A response containing the article summary list.</returns>
        [HttpGet("{section}")]
        public async Task<Response.GetArticleList> GetArticleList(uint section, [FromQuery(Name = "offset")] ushort offset)
        {
            await using var conn = _dbContext.Connection(-1);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("section", section);
            dynamicParams.Add("position", offset);
            var articles = await conn.QueryAsync<Http.Model.Bulletin>($"USP_BULLETIN_GET_LIST", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);

            var summaryList = _mapper.Map<List<Http.Model.Bulletin>, List<ArticleSummary>>(articles.ToList());
            return new Response.GetArticleList
            {
                SummaryList = summaryList
            };
        }

        /// <summary>
        /// Retrieves a specific article by section and article ID.
        /// Returns the full article content and navigation information.
        /// </summary>
        /// <param name="section">The bulletin section ID containing the article.</param>
        /// <param name="id">The unique identifier of the article.</param>
        /// <returns>A response containing the article content and navigation information, or error details.</returns>
        [HttpGet("{section}/{id}")]
        public async Task<Response.GetArticle> GetArticle(uint section, ushort id)
        {
            try
            {
                await using var conn = _dbContext.Connection(-1);
                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("section", section);
                dynamicParams.Add("article", id);
                await using var reader = await conn.QueryMultipleAsync($"USP_BULLETIN_GET", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);
                var article = await reader.ReadFirstOrDefaultAsync<Http.Model.Bulletin>() ??
                    throw new LogicException(Fb.Model.EnumValue.ErrorCode.ArticleNotExists);

                var next = await reader.ReadFirstAsync<bool>();
                return new Response.GetArticle
                {
                    Article = _mapper.Map<fb.protocol._internal.Article>(article),
                    Next = next,
                    Success = true,
                };
            }
            catch (Exception)
            {
                return new Response.GetArticle
                {
                    Success = false
                };
            }
        }

        /// <summary>
        /// Handles article writing requests to create new bulletin posts.
        /// Creates a new article in the specified bulletin section using a stored procedure.
        /// </summary>
        /// <param name="request">The article writing request containing section, user, title, and content.</param>
        /// <returns>A response indicating the success of article creation.</returns>
        [HttpPost("write")]
        public async Task<Response.WriteArticle> Write(Request.WriteArticle request)
        {
            await using var conn = _dbContext.Connection(-1);
            var dynamicParams = new DynamicParameters();
            dynamicParams.Add("section", request.Section);
            dynamicParams.Add("uid", request.User);
            dynamicParams.Add("title", request.Title);
            dynamicParams.Add("contents", request.Contents);
            var result = await conn.ExecuteAsync($"USP_BULLETIN_ADD", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);

            return new Response.WriteArticle
            {
                Success = result == 1
            };
        }

        /// <summary>
        /// Handles article deletion requests for removing bulletin posts.
        /// Deletes the specified article if the user has appropriate permissions.
        /// </summary>
        /// <param name="request">The article deletion request containing article ID and user ID.</param>
        /// <returns>A response with the deletion result code.</returns>
        [HttpPost("delete")]
        public async Task<Response.DeleteArticle> Delete(Request.DeleteArticle request)
        {
            try
            {
                await using var conn = _dbContext.Connection(-1);
                var dynamicParams = new DynamicParameters();
                dynamicParams.Add("id", request.Id);
                dynamicParams.Add("user", request.User);
                var result = await conn.ExecuteScalarAsync<int>($"USP_BULLETIN_DELETE", dynamicParams, commandType: System.Data.CommandType.StoredProcedure);

                return new Response.DeleteArticle
                {
                    Result = result
                };
            }
            catch (LogicException e)
            {
                return new Response.DeleteArticle
                {
                    Result = (int)e.Error
                };
            }
            catch (Exception)
            {
                return new Response.DeleteArticle
                {
                    Result = (int)ErrorCode.Unhandled
                };
            }
        }
    }
}