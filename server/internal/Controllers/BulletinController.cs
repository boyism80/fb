using AutoMapper;
using fb.protocol._internal;
using Fb.Model.EnumValue;
using Http;
using Http.Service;
using Microsoft.AspNetCore.Mvc;
using Request = fb.protocol._internal.request;
using Response = fb.protocol._internal.response;

namespace Internal.Controllers
{
    [ApiController]
    [Route("bulletin")]
    public class BulletinController : ControllerBase
    {
        private readonly IConfiguration _configuration;
        private readonly IMapper _mapper;
        private readonly DbContext _dbContext;
        private readonly BulletinService _bulletinService;
        private readonly ILogger<BulletinController> _logger;

        public BulletinController(
            IConfiguration configuration,
            IMapper mapper,
            DbContext dbContext,
            BulletinService bulletinService,
            ILogger<BulletinController> logger)
        {
            _configuration = configuration;
            _mapper = mapper;
            _dbContext = dbContext;
            _bulletinService = bulletinService;
            _logger = logger;
        }

        [HttpGet("{world}/{section}")]
        public async Task<Response.GetArticleList> GetArticleList(uint world, uint section, [FromQuery(Name = "offset")] ushort offset)
        {
            var articleList = await _bulletinService.GetArticleListAsync(world, section, offset);

            var summaryList = _mapper.Map<List<Http.Model.Bulletin>, List<ArticleSummary>>(articleList);
            return new Response.GetArticleList
            {
                SummaryList = summaryList
            };
        }

        [HttpGet("{world}/{section}/{id}")]
        public async Task<Response.GetArticle> GetArticle(uint world, uint section, ushort id)
        {
            try
            {
                var (article, next) = await _bulletinService.GetArticleAsync(world, section, id);

                if (article == null)
                {
                    throw new LogicException(Fb.Model.EnumValue.ErrorCode.ArticleNotExists);
                }

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

        [HttpPost("write")]
        public async Task<Response.WriteArticle> Write(Request.WriteArticle request)
        {
            try
            {
                var success = await _bulletinService.Write(
                    request.World,
                    request.Section,
                    request.User,
                    request.Title,
                    request.Contents
                );

                return new Response.WriteArticle
                {
                    Success = success
                };
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error enqueueing bulletin write");
                return new Response.WriteArticle
                {
                    Success = false
                };
            }
        }

        [HttpPost("delete")]
        public async Task<Response.DeleteArticle> Delete(Request.DeleteArticle request)
        {
            try
            {
                var result = await _bulletinService.Delete(
                    request.World,
                    request.Section,
                    request.Id,
                    request.User
                );

                return new Response.DeleteArticle
                {
                    Result = result
                };
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error enqueueing bulletin delete");
                return new Response.DeleteArticle
                {
                    Result = (int)ErrorCode.Unhandled
                };
            }
        }
    }
}